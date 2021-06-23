cat *.log | grep -P ".* [c|d]tor.*" |\
perl -pe 's/^\[.*\]//' |\
awk -F' ' '{if($2 == "ctor") {sum[$1]+=1; ctors[$1] += 1;} else if($2 == "dtor") {sum[$1]-=1; dtors[$1] += 1;} } END {for(i in sum) {printf "%5d %5d %5d %s \n", sum[i], ctors[i], dtors[i], i}}' | sort -rnb