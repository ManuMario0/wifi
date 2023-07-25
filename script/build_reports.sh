search_dir=../../users/
date=$(date "+%Y-%m-%dT%H:%M:%SZ")

cd reportGen

index=0
string=

for entry in `ls $search_dir`; do
    string="${string}resume_generator.py $entry $date\n"
	index=$(expr $index + 1)
	if [ $(expr $index % 4) -eq 0 ]
	then
		echo $string | xargs -n3 -P4 python3
		string=
		index=0
	fi
done

cd ..
