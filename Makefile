all:
	@echo "Compile sense-hat-socket"
	mkdir -p bin/
	g++ -lRTIMULib src/sense-hat-socket.cpp -o bin/sense-hat-socket
	@echo "Compile sense-hat-socket SUCCESS"

install:
	@echo "Install sense-hat-socket"
	systemctl stop sense-hat-socket.service || true
	cp bin/sense-hat-socket /usr/lib/sense-hat-socket
	cp sense-hat-socket.service /etc/systemd/system/sense-hat-socket.service
	systemctl enable sense-hat-socket.service
	systemctl restart sense-hat-socket.service
	@echo "Install sense-hat-socket SUCCESS"

uninstall:
	@echo "Uninstall sense-hat-socket"
	systemctl stop sense-hat-socket.service || true
	rm -f /usr/lib/sense-hat-socket
	rm -f /etc/systemd/system/sense-hat-socket.service
	@echo "Uninstall sense-hat-socket SUCCESS"

clean:
	rm -rf bin/
