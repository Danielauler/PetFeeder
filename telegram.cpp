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

void sqwv(int pin, int degree, int N)
{
    int t1 = (100 * degree + 4) / 9 + 1500;
    int t2 = 20000 - t1;
    int i;
    for (i = 0; i < N; i++)
    {
        digitalWrite(pin, HIGH);
        usleep(t1);
        digitalWrite(pin, LOW);
        usleep(t2);
    }
}

void feederFunction(int delayTime, int N)
{
    sqwv(SERVO, 90, N);
    sleep(delayTime);
    sqwv(SERVO, 0, N);
};

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
    
    pinMode(SERVO, OUTPUT);
    sqwv(SERVO, 0, N);
    Bot bot("931015860:AAHG6qZTMlopgG29lXC6-_rAPSmNrKiYXm4");

    InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
    vector<InlineKeyboardButton::Ptr> row0;
    InlineKeyboardButton::Ptr checkButton(new InlineKeyboardButton);
    InlineKeyboardButton::Ptr checkButton2(new InlineKeyboardButton);
    checkButton->text = "alimentar";
    checkButton->callbackData = "alimentar";
    row0.push_back(checkButton);
    keyboard->inlineKeyboard.push_back(row0);
    vector<InlineKeyboardButton::Ptr> row1;
    checkButton2->text = "agendar uma refeição";
    checkButton2->callbackData = "agendar";
    row1.push_back(checkButton2);
    keyboard->inlineKeyboard.push_back(row1);

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Olá, vou te ajudar a manter seu pet alimentado. Use o comando /help para mais informações");
    });

    bot.getEvents().onCommand("alimentar", [&bot](Message::Ptr message) {
        thread feeder(feederFunction, 2, 40);
        feeder.join();
        bot.getApi().sendMessage(message->chat->id, "Alimentado");
    });

    bot.getEvents().onCommand("agendar", [&bot, &keyboard](Message::Ptr message) {
        string response = "ok";
        bot.getApi().sendMessage(message->chat->id, response, false, 0, keyboard, "Markdown");
    });

    bot.getEvents().onCommand("help", [&bot, &keyboard](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Você pode: ", false, 0, keyboard);
    });

    bot.getEvents().onCallbackQuery([&bot, &keyboard](CallbackQuery::Ptr query) {
        if (StringTools::startsWith(query->data, "alimentar"))
        {
            thread feeder(feederFunction, 2, 40);
            feeder.join();
            string response = "Ok, alimentado";
            bot.getApi().sendMessage(query->message->chat->id, response);
        }
    });
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        unsigned char user_input=1, send_msp430;
        string userMessage = message->text.c_str();
        &user_input = userMessage;
        if((user_input<0) || (user_input>5))
			puts("Valor invalido");
		else if(user_input>0)
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