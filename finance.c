/* finance.c
   This file does the money calculations for SmartSpend.
   It also handles payment dates, reminders and the savings goal.
*/

#include <stdio.h>
#include <string.h>
#include "smartspend.h"

/* Add the amount of every income transaction. */
float totalIncome(void) {
    float total = 0;
    int i;

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].isIncome == 1) {
            total = total + transactions[i].amount;
        }
    }

    return total;
}

/* Add the amount of every expense transaction. */
float totalExpenses(void) {
    float total = 0;
    int i;

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].isIncome == 0) {
            total = total + transactions[i].amount;
        }
    }

    return total;
}

/* Balance means total income minus total expenses. */
float balance(void) {
    float income = totalIncome();
    float expenses = totalExpenses();

    return income - expenses;
}

/* Find all expenses made in the selected month and year. */
float spentInMonth(int month, int year) {
    float total = 0;
    int i;

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].isIncome == 0) {
            if (transactions[i].date.month == month &&
                transactions[i].date.year == year) {
                total = total + transactions[i].amount;
            }
        }
    }

    return total;
}

/* Find expenses for one category in the selected month. */
float spentInCategory(int category, int month, int year) {
    float total = 0;
    int i;

    for (i = 0; i < transactionCount; i++) {
        if (transactions[i].isIncome == 0) {
            if (transactions[i].category == category &&
                transactions[i].date.month == month &&
                transactions[i].date.year == year) {
                total = total + transactions[i].amount;
            }
        }
    }

    return total;
}

/* Count the days from 1 January 2000 up to the given date.
   This makes it easier to find the number of days between two dates. */
int dayNumber(Date date) {
    int totalDays = date.day;
    int year;
    int month;

    for (year = 2000; year < date.year; year++) {
        totalDays = totalDays + 365;

        if (isLeapYear(year) == 1) {
            totalDays = totalDays + 1;
        }
    }

    for (month = 1; month < date.month; month++) {
        totalDays = totalDays + getDaysInMonth(month, date.year);
    }

    return totalDays;
}

/* Return the number of days from today until the given date. */
int daysUntil(Date date) {
    int paymentDayNumber = dayNumber(date);
    int todayDayNumber = dayNumber(today());

    return paymentDayNumber - todayDayNumber;
}

/* Move a date forward by one month. */
Date nextMonth(Date date) {
    int lastDay;

    date.month = date.month + 1;

    if (date.month > 12) {
        date.month = 1;
        date.year = date.year + 1;
    }

    lastDay = getDaysInMonth(date.month, date.year);

    /* Example: 31 January becomes 28 February (or 29 in a leap year). */
    if (date.day > lastDay) {
        date.day = lastDay;
    }

    return date;
}

/* Move overdue monthly payments to their next due date. */
void rollRecurring(void) {
    int i;

    for (i = 0; i < paymentCount; i++) {
        while (paymentList[i].repeatsMonthly == 1 &&
               daysUntil(paymentList[i].dueDate) < 0) {
            paymentList[i].dueDate = nextMonth(paymentList[i].dueDate);
        }
    }

    sortPayments();
}

/* Mark a payment as paid and save it as a Bills expense. */
void markPaid(int index) {
    if (index < 0 || index >= paymentCount) {
        return;
    }

    addTransaction(today(), 4, paymentList[index].name,
                   paymentList[index].amount, 0);

    if (paymentList[index].repeatsMonthly == 1) {
        paymentList[index].dueDate = nextMonth(paymentList[index].dueDate);
        rollRecurring();
    } else {
        deletePayment(index);
    }
}

/* Add payments due within the next number of days.
   Overdue payments are also included. */
float upcomingPayments(int withinDays) {
    float total = 0;
    int i;

    for (i = 0; i < paymentCount; i++) {
        int daysLeft = daysUntil(paymentList[i].dueDate);

        if (daysLeft <= withinDays) {
            total = total + paymentList[i].amount;
        }
    }

    return total;
}

/* Work out what percentage of the savings goal is complete. */
float goalPercent(void) {
    float percent;

    if (savingsGoal.targetAmount <= 0) {
        return 0;
    }

    percent = savingsGoal.savedAmount / savingsGoal.targetAmount * 100;

    if (percent > 100) {
        percent = 100;
    }

    return percent;
}

/* Work out how many months are needed to reach the savings goal. */
float monthsToGoal(float savingPerMonth) {
    float moneyStillNeeded;

    if (savingPerMonth <= 0) {
        return 0;
    }

    moneyStillNeeded = savingsGoal.targetAmount - savingsGoal.savedAmount;

    if (moneyStillNeeded <= 0) {
        return 0;
    }

    return moneyStillNeeded / savingPerMonth;
}

/* Add one message to the alerts list. */
void addAlert(char message[]) {
    if (alertCount >= MAX_ALERTS) {
        return;
    }

    snprintf(alerts[alertCount], sizeof(alerts[alertCount]), "%s", message);
    alertCount = alertCount + 1;
}

/* Check budgets, payments, savings and reserve, then make alerts. */
void makeAlerts(void) {
    Date currentDate = today();
    char message[100];
    float monthlySpending;
    int i;

    alertCount = 0;
    monthlySpending = spentInMonth(currentDate.month, currentDate.year);

    /* Check the complete monthly budget. */
    if (monthlyBudget > 0) {
        if (monthlySpending > monthlyBudget) {
            addAlert("Monthly budget exceeded!");
        } else if (monthlySpending >= monthlyBudget * 0.8) {
            addAlert("You are approaching your monthly spending limit.");
        }
    }

    /* Check each category budget. */
    for (i = 0; i < NUM_CATEGORIES; i++) {
        float categorySpending = spentInCategory(i, currentDate.month, currentDate.year);

        if (categoryBudget[i] > 0) {
            if (categorySpending > categoryBudget[i]) {
                sprintf(message, "Your %s expenses have exceeded the planned budget.",
                        categoryNames[i]);
                addAlert(message);
            } else if (categorySpending >= categoryBudget[i] * 0.8) {
                sprintf(message, "%s budget is nearly used up.", categoryNames[i]);
                addAlert(message);
            }
        }
    }

    /* Check when each payment is due. */
    for (i = 0; i < paymentCount; i++) {
        int daysLeft = daysUntil(paymentList[i].dueDate);

        if (daysLeft < 0) {
            sprintf(message, "%s - Rs. %.0f - OVERDUE by %d day(s)",
                    paymentList[i].name, paymentList[i].amount, -daysLeft);
            addAlert(message);
        } else if (daysLeft == 0) {
            sprintf(message, "%s - Rs. %.0f - due TODAY",
                    paymentList[i].name, paymentList[i].amount);
            addAlert(message);
        } else if (daysLeft <= 7) {
            sprintf(message, "%s - Rs. %.0f - due in %d day(s)",
                    paymentList[i].name, paymentList[i].amount, daysLeft);
            addAlert(message);
        }
    }

    /* Remind the user if the savings goal is not complete. */
    if (savingsGoal.targetAmount > 0 &&
        savingsGoal.savedAmount < savingsGoal.targetAmount) {
        sprintf(message, "Savings goal '%s' is %.0f%% complete.",
                savingsGoal.name, goalPercent());
        addAlert(message);
    }

    /* Check whether the balance can cover the emergency reserve. */
    if (balance() < reserveAmount) {
        addAlert("Balance is below your reserve amount.");
    }
}

/* Give a simple score based on the user's financial position. */
int healthScore(void) {
    Date currentDate = today();
    int score = 100;
    int i;

    if (monthlyBudget > 0 &&
        spentInMonth(currentDate.month, currentDate.year) > monthlyBudget) {
        score = score - 25;
    }

    for (i = 0; i < NUM_CATEGORIES; i++) {
        float categorySpending = spentInCategory(i, currentDate.month, currentDate.year);

        if (categoryBudget[i] > 0 && categorySpending > categoryBudget[i]) {
            score = score - 8;
        }
    }

    if (balance() < reserveAmount) {
        score = score - 20;
    }

    if (balance() < upcomingPayments(30)) {
        score = score - 15;
    }

    if (savingsGoal.targetAmount > 0 && goalPercent() < 50) {
        score = score - 10;
    }

    for (i = 0; i < paymentCount; i++) {
        if (daysUntil(paymentList[i].dueDate) < 0) {
            score = score - 5;
        }
    }

    if (score < 0) {
        score = 0;
    }

    return score;
}

/* Return 1 when the purchase is affordable, otherwise return 0.
   The user must still have money for upcoming payments and the reserve. */
int canAfford(float price) {
    float moneyLeft = balance();

    moneyLeft = moneyLeft - upcomingPayments(30);
    moneyLeft = moneyLeft - reserveAmount;
    moneyLeft = moneyLeft - price;

    if (moneyLeft >= 0) {
        return 1;
    }

    return 0;
}
