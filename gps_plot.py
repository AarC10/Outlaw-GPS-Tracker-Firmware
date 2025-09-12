#!/usr/bin/env python3
import argparse
import re
import threading
import queue
import time
from dataclasses import dataclass
from typing import Optional, List

import serial 
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Mapping bits
import contextily as cx        
from pyproj import Transformer

# ---------- Parsing ----------
PACKET_START_RE = re.compile(r"Packet received")
LAT_RE = re.compile(r"Latitude:\s*([-]?\d+)")
LON_RE = re.compile(r"Longitude:\s*([-]?\d+)")
BEARING_RE = re.compile(r"Bearing:\s*([-]?\d+)")
SPEED_RE = re.compile(r"Speed:\s*([-]?\d+)")
ALT_RE = re.compile(r"Altitude:\s*([-]?\d+)")

def nano_to_deg(n: int) -> float:
    return n / 1e9

@dataclass
class Fix:
    t: float
    lat_deg: float
    lon_deg: float
    bearing_md: Optional[int]
    speed_raw: Optional[int]
    alt_raw: Optional[int]
    x: Optional[float] = None
    y: Optional[float] = None

class PacketBuild:
    def __init__(self):
        self.lat_nano = None
        self.lon_nano = None
        self.bearing = None
        self.speed = None
        self.alt = None

    def complete(self) -> bool:
        return self.lat_nano is not None and self.lon_nano is not None

    def to_fix(self) -> Fix:
        return Fix(
            t=time.time(),
            lat_deg=nano_to_deg(int(self.lat_nano)),
            lon_deg=nano_to_deg(int(self.lon_nano)),
            bearing_md=(None if self.bearing is None else int(self.bearing)),
            speed_raw=(None if self.speed is None else int(self.speed)),
            alt_raw=(None if self.alt is None else int(self.alt)),
        )

class SerialReader(threading.Thread):
    def __init__(self, port: str, baud: int, out_q: queue.Queue):
        super().__init__(daemon=True)
        self.port = port
        self.baud = baud
        self.out_q = out_q
        self._stop = threading.Event()

    def run(self):
        try:
            with serial.Serial(self.port, self.baud, timeout=0.25) as ser:
                buf = PacketBuild()
                while not self._stop.is_set():
                    try:
                        line = ser.readline().decode(errors="ignore").strip()
                    except Exception:
                        continue
                    if not line:
                        continue

                    if PACKET_START_RE.search(line):
                        if buf.complete():
                            self.out_q.put(buf.to_fix())
                        buf = PacketBuild()
                        continue

                    m = LAT_RE.search(line)
                    if m:
                        buf.lat_nano = int(m.group(1)); continue
                    m = LON_RE.search(line)
                    if m:
                        buf.lon_nano = int(m.group(1)); continue
                    m = BEARING_RE.search(line)
                    if m:
                        buf.bearing = int(m.group(1)); continue
                    m = SPEED_RE.search(line)
                    if m:
                        buf.speed = int(m.group(1)); continue
                    m = ALT_RE.search(line)
                    if m:
                        buf.alt = int(m.group(1)); continue

                if buf.complete():
                    self.out_q.put(buf.to_fix())
        except serial.SerialException as e:
            print(f"[ERROR] Serial error on {self.port}: {e}")

    def stop(self):
        self._stop.set()

class LiveMap:
    def __init__(self, max_points=3000, initial_pad_m=500):
        self.fixes: List[Fix] = []
        self.max_points = max_points
        self.initial_pad_m = initial_pad_m
        self.transformer = Transformer.from_crs("EPSG:4326", "EPSG:3857", always_xy=True)

        self.fig = plt.figure(figsize=(9, 9))
        gs = self.fig.add_gridspec(3, 1, height_ratios=[3.5, 0.05, 1])
        self.ax = self.fig.add_subplot(gs[0, 0])   # map
        self.ax.set_title("pee")
        self.ax.set_xticks([]); self.ax.set_yticks([])

        self.trail_line, = self.ax.plot([], [], lw=2, marker=None)
        self.curr_point, = self.ax.plot([], [], marker='o', ms=8)

        # Info panel
        self.ax_info = self.fig.add_subplot(gs[2, 0]); self.ax_info.axis("off")
        self.info_text = self.ax_info.text(
            0.01, 0.98, "", va="top", ha="left", family="monospace", fontsize=10,
            transform=self.ax_info.transAxes
        )

        self._basemap_inited = False
        self._last_basemap_time = 0.0
        self._basemap_rate_limit_s = 2.0

        self.fig.canvas.mpl_connect("key_press_event", self._on_key)

    def _on_key(self, ev):
        if ev.key == 'q':
            plt.close(self.fig)

    def _ll_to_xy(self, lon, lat):
        x, y = self.transformer.transform(lon, lat)
        return float(x), float(y)

    def add_fix(self, fix: Fix):
        x, y = self._ll_to_xy(fix.lon_deg, fix.lat_deg)
        fix.x, fix.y = x, y
        self.fixes.append(fix)
        if len(self.fixes) > self.max_points:
            self.fixes = self.fixes[-self.max_points:]

    def _current_extent(self):
        xs = [f.x for f in self.fixes]
        ys = [f.y for f in self.fixes]
        if not xs or not ys:
            return None
        xmin, xmax = min(xs), max(xs)
        ymin, ymax = min(ys), max(ys)
        pad_x = max(25.0, (xmax - xmin) * 0.2)
        pad_y = max(25.0, (ymax - ymin) * 0.2)
        return (xmin - pad_x, xmax + pad_x, ymin - pad_y, ymax + pad_y)

    def _ensure_basemap(self, force=False):
        if not self.fixes:
            return
        now = time.time()
        if not force and (now - self._last_basemap_time) < self._basemap_rate_limit_s:
            return

        if not self._basemap_inited:
            # First fix: center with a reasonable pad
            x0, y0 = self.fixes[-1].x, self.fixes[-1].y
            self.ax.set_xlim(x0 - self.initial_pad_m, x0 + self.initial_pad_m)
            self.ax.set_ylim(y0 - self.initial_pad_m, y0 + self.initial_pad_m)
            cx.add_basemap(self.ax, crs="EPSG:3857", source=cx.providers.OpenStreetMap.Mapnik)
            self._basemap_inited = True
            self._last_basemap_time = now
            return

        # If new point is outside current view, expand and refresh basemap
        x, y = self.fixes[-1].x, self.fixes[-1].y
        x0, x1 = self.ax.get_xlim()
        y0, y1 = self.ax.get_ylim()
        outside = (x < x0) or (x > x1) or (y < y0) or (y > y1)
        if outside or force:
            extent = self._current_extent()
            if extent:
                self.ax.set_xlim(extent[0], extent[1])
                self.ax.set_ylim(extent[2], extent[3])
                cx.add_basemap(self.ax, crs="EPSG:3857", source=cx.providers.OpenStreetMap.Mapnik)
                self._last_basemap_time = now

    def draw(self):
        if not self.fixes:
            return
        xs = [f.x for f in self.fixes]
        ys = [f.y for f in self.fixes]

        # Basemap ensure/refresh (rate-limited)
        self._ensure_basemap()

        self.trail_line.set_data(xs, ys)
        self.curr_point.set_data([xs[-1]], [ys[-1]])

        last = self.fixes[-1]
        bearing = f"{(last.bearing_md or 0)/1000:.1f}°" if last.bearing_md is not None else "—"
        speed_ms = f"{(last.speed_raw or 0)/100:.2f} m/s" if last.speed_raw is not None else "—"
        alt_m = f"{(last.alt_raw or 0)/1000:.2f} m" if last.alt_raw is not None else "—"

        self.info_text.set_text(
            f"Last fix @ {time.strftime('%H:%M:%S', time.localtime(last.t))}\n"
            f"Lat: {last.lat_deg:.6f}  Lon: {last.lon_deg:.6f}\n"
            f"Bearing: {bearing}   Speed: {speed_ms}   Alt: {alt_m}\n"
            f"Points: {len(self.fixes)}   (press 'q' to quit)"
        )

def main():
    ap = argparse.ArgumentParser(description="Live GPS map from Zephyr-style UART logs.")
    ap.add_argument("--port", required=True, help="Serial port (e.g., /dev/ttyUSB0)")
    ap.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    ap.add_argument("--interval", type=float, default=0.5, help="Plot update period (s)")
    ap.add_argument("--max-points", type=int, default=3000, help="Trail length cap")
    ap.add_argument("--force-refresh", action="store_true",
                    help="Force basemap refresh every animation tick (slower; mostly for debugging)")
    args = ap.parse_args()

    qfix = queue.Queue()
    sr = SerialReader(args.port, args.baud, qfix)
    sr.start()

    live = LiveMap(max_points=args.max_points)

    def on_timer(_frame):
        drained = 0
        try:
            while True:
                fix = qfix.get_nowait()
                live.add_fix(fix)
                drained += 1
        except queue.Empty:
            pass

        if drained > 0 or args.force_refresh:
            if args.force_refresh:
                live._ensure_basemap(force=True)
            live.draw()

    ani = FuncAnimation(live.fig, on_timer, interval=int(args.interval * 1000))
    try:
        plt.show()
    finally:
        sr.stop()
        sr.join(timeout=1.0)

if __name__ == "__main__":
    main()
