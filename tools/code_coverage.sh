# !/bin/bash

# need compile option: -fprofile-instr-generate -fcoverage-mapping     

if [ $# -lt 1 ]; then
	echo -e "\033[31m[ERROR]\033[0m Too few arguments"
	echo "usage: codecov <binary_file> [sources]"
	exit 1;
fi

test=`pwd`/$1

if [ ! -f ${test} ]; then
    echo -e "\033[31m[ERROR]\033[0m binary file '${test}' doesn't exist."
    exit 1; 
fi

srclist=${@}
echo ${srclist}
noexec(){
if [ ! -f ${src} ]; then
    echo -e "\033[31m[ERROR]\033[0m source file '${src}' doesn't exist."
    exit 1; 
fi
}

profraw=`basename ${test}.profraw`
echo -e "\033[33m[INFO]\033[0m profraw: ${profraw}"

args="--gtest_repeat=30"
${test} ${args} # || exit 1;
mv default.profraw ${test}.profraw || exit 1;
profdata=`basename ${test}.profdata`

echo -e "\033[33m[INFO]\033[0m llvm-profdata merge -o ${profdata}  ${profraw}"
llvm-profdata merge -o ${profdata}  ${profraw} || exit 1;

echo -e "\033[33m[INFO]\033[0m llvm-cov show -instr-profile=${profdata} ${srclist} > ${test}.detail"

for src in ${srclist[@]}; do 
	if [ -f ${src} ]; then
	    llvm-cov show -instr-profile=${profdata} ${test} ${src} > ${src}.detail || exit 1;
#	(grep "      0|" ${test}.detail > ${src}.miss) || exit 1;
#		echo -e "\033[33m[INFO]\033[0m missing region of '${src}' is saved as:  ${src}.miss"
	fi
done 

echo -e "\033[33m[INFO]\033[0m llvm-cov report ${test} -instr-profile=${profdata} ${srclist} > ${test}.code_cov;"
date >> ${test}.code_cov
llvm-cov report ${test} -instr-profile=${profdata} ${srclist} >> ${test}.code_cov || exit 1;
echo >> ${test}.code_cov

echo -e "\033[33m[INFO]\033[0m rm ${profdata} ${profraw}"
rm ${profdata} ${profraw}

echo -e "\033[32m[FINISH]\033[0m summary report output: ${test}.code_cov"
