#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
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

    system("fswebcam 320x240 foto_img.jpg");
}

bool verifyBowl(string photoFilePath)
{
    cout << "Verificando tigela" << endl;

    bool haveFood;
    // system("rm -rf foto_img.jpg")
    // thread takePhoto(takePic);
    // takePhoto.join();

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

    unsigned char user_input;
    unsigned char time_default = (unsigned char)feed_default;

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
    vector<InlineKeyboardButton::Ptr> row0;
    InlineKeyboardButton::Ptr checkButton(new InlineKeyboardButton);
    InlineKeyboardButton::Ptr checkButton2(new InlineKeyboardButton);
    InlineKeyboardButton::Ptr checkButton3(new InlineKeyboardButton);
    InlineKeyboardButton::Ptr checkButton4(new InlineKeyboardButton);

    vector<InlineKeyboardButton::Ptr> row1;

    checkButton->text = "pouco";
    checkButton->callbackData = "alimentar 1";
    row0.push_back(checkButton);
    keyboard->inlineKeyboard.push_back(row0);
    checkButton2->text = "médio";
    checkButton2->callbackData = "alimentar 2";
    row0.push_back(checkButton2);
    keyboard->inlineKeyboard.push_back(row0);

    checkButton3->text = "bastante";
    checkButton3->callbackData = "alimentar3";
    row0.push_back(checkButton3);
    keyboard->inlineKeyboard.push_back(row0);

    checkButton4->text = "Cancelar";
    checkButton4->callbackData = "cancelar";
    row0.push_back(checkButton4);
    keyboard->inlineKeyboard.push_back(row1);

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Olá, vou te ajudar a manter seu pet alimentado. Use o comando /help para mais informações");
    });

    bot.getEvents().onCommand("semVerificarAlimentar", [&bot](Message::Ptr message) {
        // thread feeder(feederFunction, 2, 40);
        // feeder.join();
        bot.getApi().sendMessage(message->chat->id, "Alimentado");
    });

    bot.getEvents().onCommand("alimentar", [&bot, &photoFilePath, &photoMimeType, &keyboard](Message::Ptr message) {
        cout << message << endl;
        bot.getApi().sendMessage(message->chat->id, "Aguarde por favor!");
        bool existencia = verifyBowl(photoFilePath);
        if (!existencia)
        {
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
            sleep(1);
            bot.getApi().sendMessage(message->chat->id, response, false, 0, keyboard, "Markdown");
        }
    });

    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start"))
        {
            return;
        }
        user_input = (unsigned char)atoi(message->text.c_str());
        if ((atoi(message->text.c_str()) < 0) || (atoi(message->text.c_str()) > 5))
            puts("Valor invalido");
        else
        {
            wiringPiSPIDataRW(0, &user_input, 1);
            printf("MSP430_return = %d\n", user_input);
            sleep(1 + user_input / 2);
        }
        puts("");
        bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    });

    bot.getEvents().onCallbackQuery([&bot](CallbackQuery::Ptr query) {
        if (StringTools::startsWith(query->data, "alimentar"))
        {
            string value = query->data.c_str().substr(query->data.c_str().find(' '), query->data.c_str().find(' ') + 1);
            cout<<"value selected is: "<<value<<endl;
            user_input = (unsigned char)atoi(value);
            if ((atoi(value) < 0) || (atoi(value) > 5))
                puts("Valor invalido");
                bot.getApi().sendMessage(query->message->chat->id, "Valor invalido");
            else
            {
                wiringPiSPIDataRW(0, &user_input, 1);
                printf("MSP430_return = %d\n", user_input);
                sleep(1 + user_input / 2);
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