/* data.c
   This file stores the SmartSpend information and works with the text files.
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "smartspend.h"

/* Shared program data */
char categoryNames[NUM_CATEGORIES][20] = {
    "Food", "Transport", "Education", "Shopping",
    "Bills", "Entertainment", "Other"
};

Transaction transactions[MAX_TRANSACTIONS];
int transactionCount = 0;

Payment paymentList[MAX_PAYMENTS];
int paymentCount = 0;

float monthlyBudget = 0;
float reserveAmount = 0;
float categoryBudget[NUM_CATEGORIES];

SavingsGoal savingsGoal = {"None", 0, 0};

char alerts[MAX_ALERTS][100];
int alertCount = 0;


/* Return today's date from the computer. */
Date today(void) {
    time_t currentTime = time(NULL);
    struct tm *currentDate = localtime(&currentTime);

    Date result;
    result.day = currentDate->tm_mday;
    result.month = currentDate->tm_mon + 1;
    result.year = currentDate->tm_year + 1900;

    return result;
}


/* Convert a date written as DD/MM/YYYY into three numbers. */
Date parseDate(char text[]) {
    Date result = {0, 0, 0};

    sscanf(text, "%d/%d/%d",
           &result.day, &result.month, &result.year);

    return result;
}


/* Check whether a year is a leap year. */
int isLeapYear(int year) {
    if (year % 400 == 0) {
        return 1;
    }

    if (year % 100 == 0) {
        return 0;
    }

    if (year % 4 == 0) {
        return 1;
    }

    return 0;
}


/* Return the number of days in a month. */
int getDaysInMonth(int month, int year) {
    int days[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (month == 2 && isLeapYear(year)) {
        return 29;
    }

    if (month < 1 || month > 12) {
        return 0;
    }

    return days[month - 1];
}


/* Check that the entered date exists. */
int isValidDate(Date d) {
    if (d.year < 2000 || d.year > 2100) {
        return 0;
    }

    if (d.month < 1 || d.month > 12) {
        return 0;
    }

    if (d.day < 1 || d.day > getDaysInMonth(d.month, d.year)) {
        return 0;
    }

    return 1;
}


/* Add a new income or expense. */
int addTransaction(Date d, int category, char note[],
                   float amount, int isIncome) {
    if (transactionCount >= MAX_TRANSACTIONS) {
        return 0;
    }

    if (amount <= 0 || !isValidDate(d)) {
        return 0;
    }

    if (category < 0 || category >= NUM_CATEGORIES) {
        return 0;
    }

    transactions[transactionCount].date = d;
    transactions[transactionCount].category = category;
    transactions[transactionCount].amount = amount;
    transactions[transactionCount].isIncome = isIncome;

    snprintf(transactions[transactionCount].note,
             sizeof(transactions[transactionCount].note), "%s", note);

    if (transactions[transactionCount].note[0] == '\0') {
        strcpy(transactions[transactionCount].note, "-");
    }

    transactionCount++;
    return 1;
}


/* Add a payment to the payment list. */
int addPayment(char name[], float amount, Date dueDate, int repeatsMonthly) {
    if (paymentCount >= MAX_PAYMENTS) {
        return 0;
    }

    if (amount <= 0 || !isValidDate(dueDate)) {
        return 0;
    }

    paymentList[paymentCount].amount = amount;
    paymentList[paymentCount].dueDate = dueDate;
    paymentList[paymentCount].repeatsMonthly = repeatsMonthly;

    snprintf(paymentList[paymentCount].name,
             sizeof(paymentList[paymentCount].name), "%s", name);

    paymentCount++;
    return 1;
}


/* Delete a payment and move the remaining payments one place back. */
void deletePayment(int index) {
    if (index < 0 || index >= paymentCount) {
        return;
    }

    for (int i = index; i < paymentCount - 1; i++) {
        paymentList[i] = paymentList[i + 1];
    }

    paymentCount--;
}


/* Turn a date into a number so two dates can be compared. */
int dateKey(Date d) {
    return d.year * 10000 + d.month * 100 + d.day;
}


/* Arrange payments from the earliest date to the latest date. */
void sortPayments(void) {
    for (int i = 0; i < paymentCount - 1; i++) {
        for (int j = 0; j < paymentCount - 1 - i; j++) {
            int firstDate = dateKey(paymentList[j].dueDate);
            int secondDate = dateKey(paymentList[j + 1].dueDate);

            if (firstDate > secondDate) {
                Payment temporary = paymentList[j];
                paymentList[j] = paymentList[j + 1];
                paymentList[j + 1] = temporary;
            }
        }
    }
}


/* Save transactions, payments and settings in text files.
   The order of the values is kept the same so older saved files still work. */
void saveData(void) {
    FILE *file;

    /* Save transactions. */
    file = fopen("data_txns.txt", "w");

    if (file != NULL) {
        fprintf(file, "%d\n", transactionCount);

        for (int i = 0; i < transactionCount; i++) {
            fprintf(file, "%d %d %d %d %.2f %d %s\n",
                    transactions[i].date.day,
                    transactions[i].date.month,
                    transactions[i].date.year,
                    transactions[i].category,
                    transactions[i].amount,
                    transactions[i].isIncome,
                    transactions[i].note);
        }

        fclose(file);
    }

    /* Save payments. */
    file = fopen("data_pays.txt", "w");

    if (file != NULL) {
        fprintf(file, "%d\n", paymentCount);

        for (int i = 0; i < paymentCount; i++) {
            fprintf(file, "%d %d %d %.2f %d %s\n",
                    paymentList[i].dueDate.day,
                    paymentList[i].dueDate.month,
                    paymentList[i].dueDate.year,
                    paymentList[i].amount,
                    paymentList[i].repeatsMonthly,
                    paymentList[i].name);
        }

        fclose(file);
    }

    /* Save budgets and savings goal. */
    file = fopen("data_settings.txt", "w");

    if (file != NULL) {
        fprintf(file, "%.2f %.2f\n", monthlyBudget, reserveAmount);

        for (int i = 0; i < NUM_CATEGORIES; i++) {
            fprintf(file, "%.2f ", categoryBudget[i]);
        }

        fprintf(file, "\n%.2f %.2f\n%s\n",
                savingsGoal.targetAmount, savingsGoal.savedAmount, savingsGoal.name);

        fclose(file);
    }
}


/* Read previously saved information from the text files.
   Each fscanf reads the values written by saveData(). */
void loadData(void) {
    FILE *file;
    int savedCount;

    /* Load transactions. */
    file = fopen("data_txns.txt", "r");

    if (file != NULL) {
        if (fscanf(file, "%d", &savedCount) == 1) {
            for (int i = 0; i < savedCount && i < MAX_TRANSACTIONS; i++) {
                int result = fscanf(file, "%d %d %d %d %f %d %49[^\n]",
                                    &transactions[i].date.day,
                                    &transactions[i].date.month,
                                    &transactions[i].date.year,
                                    &transactions[i].category,
                                    &transactions[i].amount,
                                    &transactions[i].isIncome,
                                    transactions[i].note);

                if (result != 7) {
                    break;
                }

                transactionCount++;
            }
        }

        fclose(file);
    }

    /* Load payments. */
    file = fopen("data_pays.txt", "r");

    if (file != NULL) {
        if (fscanf(file, "%d", &savedCount) == 1) {
            for (int i = 0; i < savedCount && i < MAX_PAYMENTS; i++) {
                int result = fscanf(file, "%d %d %d %f %d %29[^\n]",
                                    &paymentList[i].dueDate.day,
                                    &paymentList[i].dueDate.month,
                                    &paymentList[i].dueDate.year,
                                    &paymentList[i].amount,
                                    &paymentList[i].repeatsMonthly,
                                    paymentList[i].name);

                if (result != 6) {
                    break;
                }

                paymentCount++;
            }
        }

        fclose(file);
    }

    /* Load budgets and savings goal. */
    file = fopen("data_settings.txt", "r");

    if (file != NULL) {
        fscanf(file, "%f %f", &monthlyBudget, &reserveAmount);

        for (int i = 0; i < NUM_CATEGORIES; i++) {
            fscanf(file, "%f", &categoryBudget[i]);
        }

        fscanf(file, "%f %f %29[^\n]",
               &savingsGoal.targetAmount, &savingsGoal.savedAmount, savingsGoal.name);

        fclose(file);
    }

    sortPayments();
}
