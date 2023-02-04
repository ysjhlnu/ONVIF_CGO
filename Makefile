
build:
	GOOS=linux GOARCH=amd64 CGO_ENABLE=1 go build -o onvif_cgo main.go

clean:
	rm onvif_cgo