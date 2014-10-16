lmac/test directory contains the following folders.

1. Test Driver
2. Phy_stub_module
3. Sock_bridge

1. Test driver :  Used to simulate the functionalities of UMAC, mac80211 framework.
                    This contains perlscripts directory which simulates userspace applications 
		      (like hostapd and supplicant).
                  
			The detailed description of Test Driver is provided in testdriver/README.txt file.

2. Phy_stub_module : Used to simulate the functionalities of Physical layer. 
			   This module consists of fwk_neltink_api and phy.
	* Fwk_neltink_api - is framework for netlink socket operations like nl send and nl receive.
	* Phy - This contains  the simulation of Physical layer. 
		  It receives the data from remote party and pass the information to LMAC using LMAC - PHY interface. 

3. Sock_bridge : Contains the tunnelling over network interface.
	* eth - contains tunnelling over ethernet interface.
	* wifi - contains tunnelling over wireless interface.


