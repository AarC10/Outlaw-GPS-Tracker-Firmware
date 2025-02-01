all:
	west build -b outlaw app -p auto
rm:
	west build -b radio_module app -p auto
clean:
	rm -rf build
