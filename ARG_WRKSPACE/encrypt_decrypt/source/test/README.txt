
System test setup :

        hostapd                                        uu_wlan_mactesttool.pl
          |                                                              |
          | NL Interface                                    NL Interface |   
          |                                                              |
       MAC80211 FWk                                                      |
          |                                                              |
          |                                                              |
         UMAC                                                            |
          |                       ----------------                       |
          |_______________________|     LMAC     |______phy_stub_________|
                                  |              |
                                  ----------------



script -- contains perl script to test the MAC.

To test MAC as system
    * Build umac and lmac source.
    * Build phy_stub_module/phy
    * Insert phy module and uu_wlan driver module
    * Run required perlscripts

