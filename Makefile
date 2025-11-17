all:
	west build -b outlaw_gen3 app -p auto

gen3:
	west build -b outlaw_gen3 app -p auto

gen2:
	west build -b outlaw_gen2 app -p auto

radio:
	west build -b radio_module app -p auto --build-dir build-radio-radio_mod

deputy:
	west build -b deputy receiver -p auto

gen2-rcv:
	west build -b outlaw_gen2 app -p auto -DCONFIG_DEFAULT_RECEIVE_MODE=y
radio-rcv:
	west build -b radio_module app -p auto --build-dir build-radio-radio_mod -DCONFIG_DEFAULT_RECEIVE_MODE=y

clean:
	rm -rf build build-radio-radio_mod
