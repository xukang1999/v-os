# v-os
V-OS userspace virtual operating system kernel. it support linux/macOS/Windows.
## Quick Start
    # clone v-os
    mkdir -p /data/v-os
    git clone https://github.com/starosxyz/v-os.git /data/v-os
    
    #compile
    cd /data/v-os/build
    source .env.sh
    make debug=0 static=0 os=ubuntu20/macOS
	
## Source Introduction 
	+src
		-freebsd  				freebsd 13.0.0 kernel source 
		-fstackutils  			tcp/udp utils
		-init 					init test 
		-ports 					ports code for win32/unix/linux
		-ipstacktcpclient   	tcp client for unittest
		-ipstacktcpserserver   	tcp server for unittest
## Licenses
See [LICENSE](LICENSE)
