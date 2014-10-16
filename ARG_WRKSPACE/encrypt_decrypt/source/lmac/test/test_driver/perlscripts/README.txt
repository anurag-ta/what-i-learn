
Perlscripts contains script files to test LMAC alone using the New Test setup.

New test setup:


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

This directory contains following folders:

1.captured_frames
2.config_files
3.LMAC_testcases
4.Scripts

** captured_frames --   consists of frames captured from third party tools (omnipeek,wireshark).
** config_files    --   consists of frames,corresponding rx vectors and configuration files used in test script file.
** LMAC_testcases  --  This folder consists of
                    * frameCapabiliites --  consists of information about the frame (frame header,frame body).
                    * frameInfo         --  consists of frame information structure (TX vector,AMPDU info,MPDU) for all the categories of LMAC.
                    * testcases         --  consists of testcases to test all the categories of LMAC. ( The categories of LMAC are defined in LMAC test design and procedure document).             
                    * and text files that consists of list of test cases for all the categories of LMAC.

** Scripts        --    consists of test script files (userspace applications that behaves as hostapd and supplicant).
