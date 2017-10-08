#!/bin/bash
clear
echo -e "\n\nThis in the dependency installation script of GridStore. Written by Mahmoud Mohsen 2017 (mahmoud.mohsen@ovgu.de)\n
When proceeding, required dependencies (e.g., libraries) to configure and make GridStore will be installed in your system. \n"

read -p "Do you want to proceed [Y/n]?" ans

if [ $ans == "Y" ]
	then
	    if [ "$OSTYPE" == "linux-gnu" ] 
	    then
				echo "operating system detected is linux"	    
				sleep 1
                        	echo "check if the linux distro is Ubuntu"
                        	if [ $(grep DISTRIB_ID /etc/*-release | awk -F '=' '{print $2}') == "Ubuntu" ] 
				then
						echo "Ubuntu detected"
                                                echo "Check if you already have the required packages installed"
                                                packages_to_be_installed=()
                                                packages_installed=()
                                                packages_to_be_upgraded=()
						if [ -z "$(clang --version)" ]
                                                then
							packages_to_be_installed+=("clang")
					        elif [ -z "$(apt-get -s upgrade | grep clang)" ]
					        then
						   		   packages_installed+=("clang")
                                                else
							packages_to_be_upgraded+=("clang")			        
						fi

                                                if [ -z "$(openssl version -v)" ]
                                                then
							packages_to_be_installed+=("openssl")	
					        elif [ -z "$(apt-get -s upgrade | grep openssl)"]
						then
						   	   packages_installed+=("openssl")
                                                else	
							packages_to_be_upgraded+=("openssl")			        
                                                
                                                fi

                                                if [ -z "$(dpkg -l 'libapr1-dev')" ]
                                                then
							packages_to_be_installed+=("apr")	
					        elif [ -z "$(apt-get -s upgrade | grep libapr1-dev)" ]
						then
						   		   packages_installed+=("apr")
                                                else
							packages_to_be_upgraded+=("apr")			        

                                                fi

                                                if [ -z "$(dpkg -l 'libbsd-dev')" ]
                                                then
							packages_to_be_installed+=("bsd")	
                                               
					        elif [ -z "$(apt-get -s upgrade | grep libbsd-dev)" ]
						then
						   		   packages_installed+=("bsd")
 
						else
								packages_to_be_upgraded+=("bsd")			        

					        fi

                                                if [  -z "$(dpkg -l 'libncurses5')" ]
                                                then
							packages_to_be_installed+=("libncurses5")	
   
					        elif [ -z "$(apt-get -s upgrade | grep libncurses5)" ]
								then
						   		   packages_installed+=("libncurses5")
                                                else
							 packages_to_be_upgraded+=("libncurses5")			        
                                                fi

                                                if [ -z "$(doxygen --version)" ]
                                                then
							packages_to_be_installed+=("doxygen")	
  
					        elif [ -z "$(apt-get -s upgrade | grep doxygen)" ]
						then
						   		   packages_installed+=("doxygen")
                                                else
							packages_to_be_upgraded+=("doxygen")			        
 
                                                fi
                                                echo -e  "\n\n############ packages already installed ############\n"
                                                printf '(%s) \t' "${packages_installed[@]}"
                                                sleep 5
                                                echo  -e "\n\n############ packages to be upgraded ############\n"
                                                printf '(%s) \t' "${packages_to_be_upgraded[@]}" 
						sleep 5
                                                echo  -e "\n\n############ packages to be installed ############\n"
                                                printf '(%s) \t' "${packages_to_be_installed[@]}"		
						sleep 5
                                                echo ""
                                                echo ""
                                                if [ ${#packages_installed[@]} == 6 ]
                                                then
                                                echo "you have the needed packages already installed .... Exiting the script"
                                                sleep 3
                                                exit
                                                fi         
                                                read -p "Do you want to proceed with installation? (root password will be needed) [Y/n]?" ans
					        if [ $ans == "Y" ]
							then
                                                                read -s -p "please enter your root password:" password
								echo $password | sudo -S apt-get update
                                                		value="clang"
                                                		if [[ ! " ${packages_installed[@]} " =~ " ${value} " ]]; 
								then
								echo "installing clang"
								echo $password | sudo -S apt-get install clang    			
								fi
                                                		value="openssl"
                                                		if [[ ! " ${packages_installed[@]} " =~ " ${value} " ]]; 
								then
									echo "installing the openssl library"
									echo $password | sudo -S install openssl
									echo $password | sudo -S apt-get install libssl-dev
								fi
                                                		value="apr"
                                                		if [[ ! " ${packages_installed[@]} " =~ " ${value} " ]];
								then
								echo "installing the Apache Portable Runtime(APR) library"
								echo $password | sudo -S apt-get install apache2-dev libapr1-dev libaprutil1-dev
								fi
                                                		value="bsd"
                                                		if [[ ! " ${packages_installed[@]} " =~ " ${value} " ]];
								then
									echo "installing the freebsd library"
									echo $password | sudo -S apt-get install libbsd-dev
								fi
                                                		value="libncurses5"  
                                                		if [[ ! " ${packages_installed[@]} " =~ " ${value} " ]]; 
								then
								       echo "installing curses"
								       echo $password | sudo -S apt-get install libncurses5-dev libncursesw5-dev
								fi
                                                		value="doxygen"
                                                			if [[ ! " ${packages_installed[@]} " =~ " ${value} " ]] 
								then
									echo "installing doxygen"
									echo $password | sudo -S apt-get install doxygen
								fi
								echo -e "\nchecking the versions of the libraries"
								echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
								echo -e "check for the openssl\n"
                                                		sleep 1 
								echo -e "the version we are using is OpenSSL 1.0.2g  1 Mar 2016 \n"
								echo "the version you have installed is $(openssl version -v)"
								sleep 8						
								echo -e  "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
								echo -e  "check for the apr\n"
								sleep 1							
								echo -e "the version we are using is 1.5.2-3\n"
								echo "the version you have installed is $(dpkg -l 'libapr1-dev')"
								sleep 8						
								echo -e  "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
								echo -e  "check for the bsd\n"
								sleep 1
								echo -e "the version we are using is 0.8.2-1\n"
								echo "the version you have installed is $(dpkg -l 'libbsd-dev')"
								sleep 8						
								echo -e "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
								echo -e "check for clang\n"
								sleep 1						
								echo -e "the version we are using is 3.8.0\n"
								echo "the version you have installed is $(clang --version)"
								sleep 8							
								echo -e  "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
								echo -e  "check for the Curses\n"
								sleep 1							
								echo -e "the version we are using is 6.0+20160213\n"
								echo "the version you have installed is $(dpkg -l 'libncurses5')"
								sleep 8							
								echo -e "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
								echo -e "check for the Doxygen\n"
								sleep 1							
								echo -e "the version we are using is 1.8.11\n"
								echo "the version you have installed is $(doxygen --version)"
								sleep 8						
								echo -e "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"

                                                fi
						else 
							echo "Your linux distribution is not supported.....exiting"
                                        		sleep 2
							exit
	 fi
    elif [ "$OSTYPE" == "darwin"* ]
    then
        echo " OS code goes here " 
    fi

fi	

