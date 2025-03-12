all:
	west build -b outlaw app -p auto
radio:
	west build -b radio_module app -p auto --build-dir build-radio-radio_mod

outlaw-rcv:
	west build -b outlaw app -p auto -DCONFIG_DEFAULT_RECEIVE_MODE=y
radio-rcv:
	west build -b radio_module app -p auto --build-dir build-radio-radio_mod -DCONFIG_DEFAULT_RECEIVE_MODE=y

clean:
	rm -rf build build-radio-radio_mod
