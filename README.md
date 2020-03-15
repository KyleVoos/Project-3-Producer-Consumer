# Project-3-Producer-Consumer
## Assignment for CS 460 (Operating System) at WSU-V  
## By: Team 13  
### Kyle Voos  
### Tam Nguyen  
### Muhannad Qaisi  
## Goal:
```
This project should be programmed entirely in C and must compile and run on the lab’s
Linux environment. Copying & pasting code from other teams or from the web is considered
cheating and results in an automatic F.
In this programming assignment, you will learn how to use semaphores and mutex to implement the producer-consumer problem as specified by Chapter 5: Programming Projects:
Project 3
```
  

## How to run:
1. Compile the program (if haven't already): ```make all```  
2. execute the program: ```./p3 [how long to run for] [# of producer threads] [# of consumer threads]```  
Or run ```make test`` for a quick test run  
**Note: If you want to abort the program, use CTRL+c**
  

## Exaple of input & output:  
```
$ ./main 20 3 2
producer produced 424238335
consumer consumed 424238335
producer produced 596516649
consumer consumed 596516649
producer produced 1350490027
consumer consumed 1350490027
producer produced 2044897763
...
All threads closed.
```