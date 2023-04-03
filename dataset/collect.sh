#/bin/bash
for i in {1..5000}
do
    sleep 0.5
    python3 tweet.py >> tweet.json
done