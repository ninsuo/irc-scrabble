
SRCS	   =	main.cpp		\
				MyThread.cpp	\
				MyUser.cpp		\
				MyINI.cpp		\
				CliIRC.cpp		\
				ChannelIRC.cpp	\
				UserIRC.cpp		\
				MyString.cpp	\
				MyTimer.cpp		\
				Timer.cpp		\
				Game.cpp		\
				Scrabble.cpp

ARCHI	   =	`uname`

NAME	   =	robot.bin

OBJS	   =	${SRCS:.cpp=.o}

CPPFLAGS   =	-W -Wall -pedantic -pipe -fpermissive

CC	   =	g++

LIBS		= -lpthread

${NAME}:	${OBJS}
			g++ -o ${NAME} ${OBJS} ${LIBS}

all:		${NAME}

clean:
			rm -rf ${OBJS} *~ \#*\# *.core

fclean:		clean
			rm -rf ${NAME}

re:			fclean all
