./bin/tpcleader 1060 2 2 &
sleep 5
./bin/tpcfollower 16201 1060 &
sleep 5
./bin/tpcfollower 16202 1060 &
sleep 5
