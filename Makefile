all:
	west build -b outlaw app -p auto --build-dir build-outlaw
radio:
	west build -b radio_module app -p auto --build-dir build-radio-radio_mod
clean:
	rm -rf build-outlaw build-radio-radio_mod
