#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <tgbot/tgbot.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define SERVO 1

using namespace std;
using namespace TgBot;
int spi_fd;

int main()
{
    int N = 40;
    if(wiringPiSetup() == -1)
	{
		puts("Erro em wiringPiSetup().");
		return -1;
	}
	spi_fd = wiringPiSPISetup (0, 500000);
	if(spi_fd==-1)
	{
		puts("Erro abrindo a SPI. Garanta que ela nao");
		puts("esteja sendo usada por outra aplicacao.");
		return -1;
	}
    
    Bot bot("931015860:AAHG6qZTMlopgG29lXC6-_rAPSmNrKiYXm4");

    InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
    vector<InlineKeyboardButton::Ptr> row0;
    InlineKeyboardButton::Ptr checkButton(new InlineKeyboardButton);
    InlineKeyboardButton::Ptr checkButton2(new InlineKeyboardButton);
    checkButton->text = "alimentar";
    checkButton->callbackData = "alimentar";
    row0.push_back(checkButton);
    keyboard->inlineKeyboard.push_back(row0);

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Olá, vou te ajudar a manter seu pet alimentado. Use o comando /help para mais informações");
    });

    bot.getEvents().onCommand("alimentar", [&bot](Message::Ptr message) {
        // thread feeder(feederFunction, 2, 40);
        // feeder.join();
        bot.getApi().sendMessage(message->chat->id, "Alimentado");
    });

    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        unsigned char send_msp430;
        const unsigned char* user_input = reinterpret_cast<const unsigned char *>( message->text.c_str() );
        // user_input = message->text.c_str();
        if((strcmp(user_input,"0")==0) || (strcmp(user_input,"5")==0))
			puts("Valor invalido");
		else 
		{
			send_msp430 = user_input;
			wiringPiSPIDataRW(0, &send_msp430, 1);
			printf("MSP430_return = %d\n", send_msp430);
			sleep(1+user_input/2);
		}
		puts("");
        bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    });


    signal(SIGINT, [](int s) {
        printf("SIGINT got\n");
        exit(0);
    });

    try
    {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();
        TgLongPoll longPoll(bot);
        while (true)
        {
            printf("Long poll started\n");
            longPoll.start();
        }
    }
    catch (TgException &e)
    {
        printf("error: %s\n", e.what());
    }
    close(spi_fd);
    return 0;
}