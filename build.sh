#! /bin/bash


function build_all()
{
	echo "build all"

	if [ ! -d "${basepath}/bin" ];then
		mkdir ${basepath}/bin
	fi

	for filename in `ls ${basepath}/packages/`;
	do

		if [ -d "${basepath}/packages/${filename}" ];then
			echo "${basepath}/packages/${filename}";
			cd ${basepath}/packages/${filename} ; make;
		fi
	done

}


function build_target()
{
	echo "build ${target}"
	
	if [ ! -d "${basepath}/bin" ];then
		mkdir ${basepath}/bin
	fi
	
	cd ${basepath}/packages/${target} ; make;
}


function clean()
{
	echo "clean"
	for filename in `ls ${basepath}/packages/`;
	do

		if [ -d "${basepath}/packages/${filename}" ];then
			echo "${basepath}/packages/${filename}";
			cd ${basepath}/packages/${filename} ; make distclean;
		fi
		
		if [ -d "${basepath}/bin" ];then
			rm -rfd ${basepath}/bin
		fi
		
	done

}

basepath=$(cd `dirname $0`; pwd)
echo ${basepath}



if [ $# -eq 0 ];then
	build_all;
	exit;
fi


if [ ${1} == "clean" ];then
	clean;
	exit;
fi

target=${1}
build_target;
