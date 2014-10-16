
Test driver is to simulate the functionalities of UMAC, mac80211 framework and userspace applications (like hostapd and supplicant).

Old Test setup:

   uu_wlan_umactesttool.pl                                      uu_wlan_mactesttool.pl
          |                                                              |
          | NL Interface                                    NL Interface |   
          |                                                              |
          |                                                              |
          |                       ----------------                       |
          |____testdriver_________|     LMAC     |______phy_stub_________|
                                  |              |
                                  ----------------

New Test setup:

    
                                uu_wlan_sta_ap_script.pl 
                                        |
                                        |
          ----------------------------------------------------------------
          |                                                              |
          | NL Interface                                    NL Interface |   
          |                                                              |
          |                                                              |
          |                       ----------------                       |
          |____testdriver_________|     LMAC     |______phy_stub_________|
                                  |              |
                                  ----------------



Scripts -- contains perl script to test the LMAC using Old Test setup.

Perlscripts -- contains automation script files to test LMAC alone using the New Test setup.

Note :  Current test setup used is New Test setup.

To test LMAC alone
    * Build testdriver/umac along with the lmac source.
    * Build phy_stub_module/phy
    * Insert phy module and lmac test module
    * Run required perlscripts
    
