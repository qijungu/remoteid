# To install

1. Pull into a folder, not the ardupilot folder.

2. Move broadcast into another folder. Run "make -f Makefile.cmd" to build the broadcast sim program.

3. Copy libraries/SITL/RemoteID to the ardupilot folder. Overwrite existing files if any.

4. Compare the rest of files to the existing files in the ardupilot folder, and make changes to the exsiting files.<br>
   Recommend to use a comparison tool, such as Meld Diff Viewer, to highlight the differences of the git files and the existing files.

5. Recompile ardupilot. <br>
   Command is "./waf build" in the ardupilot folder.
   Or, in VSC, choose Terminal/Run build task/ardupilot build.

# To run

1. In the boradcast folder, run "./broadcast".

2. In the ardupilot folder, run "./build/sitl/bin/arducopter  -S -I0 --model + --speedup 1 --remoteid 616263646566303132333435363738397778797A --home 30.270083225404864,-97.7730248064704,0,90 --defaults $HOME/ardupilot/Tools/autotest/default_params/copter.parm" <br>
   -I0 : means a instance number 0. <br>
   --remoteid 616263646566303132333435363738397778797A : means the remote id of the drone is 616263646566303132333435363738397778797A. The id must have 40 hex digits. <br>

3. To launch multiple drones. <br>
   3.a. Add incremental instance numbers, such as -I1, -I2 for each drone. <br>   
   3.b. Add a unique id to each drone.

4. Run dronekit program or mavproxy. <br>
   Example of run mavproxy for two drones: mavproxy.py --master tcp:127.0.0.1:5760 --master tcp:127.0.0.1:5770 --sitl 127.0.0.1:5501 --out 127.0.0.1:14550 --out 127.0.0.1:14551 --console --map
