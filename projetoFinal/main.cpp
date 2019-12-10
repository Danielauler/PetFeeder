#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <signal.h>
#include <thread>
#include <tgbot/tgbot.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "scanBowl.h"

#define SERVO 1

using namespace std;
using namespace TgBot;

int spi_fd;
int feed_default;

void takePic()
{
    cout << "Tirando foto" << endl;

    system("fswebcam -r 320x240 foto_img.jpg");
}

bool verifyBowl(string photoFilePath)
{
    cout << "Verificando tigela" << endl;

    bool haveFood;
    system("rm -rf foto_img.jpg");
    thread takePhoto(takePic);
    takePhoto.join();

    char *tab2 = new char[photoFilePath.length() + 1];
    strcpy(tab2, photoFilePath.c_str());

    haveFood = verifyFood(tab2);
    cout << "tem ração: " << haveFood << endl;
    return haveFood;
}

int main()
{
    const string photoFilePath = "foto_img.jpg";
    const string photoMimeType = "image/jpeg";


    if (wiringPiSetup() == -1)
    {
        puts("Erro em wiringPiSetup().");
        return -1;
    }
    spi_fd = wiringPiSPISetup(0, 500000);
    if (spi_fd == -1)
    {
        puts("Erro abrindo a SPI. Garanta que ela nao");
        puts("esteja sendo usada por outra aplicacao.");
        return -1;
    }
    Bot bot("792286575:AAG0puZ_3PvXAwh6ckXb5r4-cOfn56sgibU");

    InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
    InlineKeyboardMarkup::Ptr keyboard2(new InlineKeyboardMarkup);
    vector<InlineKeyboardButton::Ptr> row0;
    InlineKeyboardButton::Ptr checkButton(new InlineKeyboardButton);
    InlineKeyboardButton::Ptr checkButton2(new InlineKeyboardButton);
    InlineKeyboardButton::Ptr checkButton3(new InlineKeyboardButton);
    InlineKeyboardButton::Ptr checkButton4(new InlineKeyboardButton);

    vector<InlineKeyboardButton::Ptr> row1;
    vector<InlineKeyboardButton::Ptr> row2;
    vector<InlineKeyboardButton::Ptr> row3;

    checkButton->text = "pouco";
    checkButton->callbackData = "nivel 1";
    row0.push_back(checkButton);
    keyboard->inlineKeyboard.push_back(row0);
    checkButton2->text = "médio";
    checkButton2->callbackData = "nivel 2";
    row0.push_back(checkButton2);
    keyboard->inlineKeyboard.push_back(row0);

    checkButton3->text = "bastante";
    checkButton3->callbackData = "nivel 3";
    row0.push_back(checkButton3);
    keyboard2->inlineKeyboard.push_back(row0);

    checkButton4->text = "Cancelar";
    checkButton4->callbackData = "cancelar";
    row2.push_back(checkButton4);
    keyboard2->inlineKeyboard.push_back(row2);

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Olá, vou te ajudar a manter seu pet alimentado. Use o comando /help para mais informações");
    });

    bot.getEvents().onCommand("semVerificarAlimentar", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Alimentado");
    });

    bot.getEvents().onCommand("alimentar", [&bot, &photoFilePath, &photoMimeType, &keyboard2](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Aguarde por favor!");
        bool existencia = verifyBowl(photoFilePath);
        if (!existencia)
        {
            unsigned char time_default = 4;
            wiringPiSPIDataRW(0, &time_default, 1);
            printf("MSP430_return = %d\n", time_default);
            sleep(1 + time_default / 2);
            string response = "Ok, alimentado";
            bot.getApi().sendMessage(message->chat->id, response);
        }
        else
        {
            string response = "Se quiser que eu complete o pote, basta me dizer o quão cheio ele já está. Ou pode cancelar!";
            bot.getApi().sendPhoto(message->chat->id, InputFile::fromFile(photoFilePath, photoMimeType), "A tigela ainda está cheia!");
            bot.getApi().sendMessage(message->chat->id, response);
        }
    });

    bot.getEvents().onCallbackQuery([&bot](CallbackQuery::Ptr query) {
        if (StringTools::startsWith(query->data, "nivel"))
        {
            unsigned char value = query->data.back();
            cout<<"value selected is: "<<value<<endl;
            if ((value < 0) || (value > 5))
            {
                puts("Valor invalido");
                bot.getApi().sendMessage(query->message->chat->id, "Valor invalido");
            }
            else
            {
                wiringPiSPIDataRW(0, &value, 1);
                printf("MSP430_return = %d\n", value);
                sleep(1 + value / 2);
            }
            puts("");
            bot.getApi().sendMessage(query->message->chat->id, "Ok, alimentado!");
        }
    });

    bot.getEvents().onCallbackQuery([&bot](CallbackQuery::Ptr query) {
        if (StringTools::startsWith(query->data, "cancel"))
        {
            string response = "ok";
            bot.getApi().sendMessage(query->message->chat->id, response);
        }
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
    return 0;
}