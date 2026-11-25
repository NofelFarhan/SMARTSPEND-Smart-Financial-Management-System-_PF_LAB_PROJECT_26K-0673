/* test_console.c
   A small test program for the SmartSpend calculations.
   It runs without opening the graphical window.
*/

#include <stdio.h>
#include <string.h>
#include "smartspend.h"

int main(void) {
    Date currentDate;
    Date invalidDate = {31, 2, 2026};
    Date paymentDate;
    int i;

    loadData();

    currentDate = today();
    paymentDate = currentDate;

    /* Add sample information only when there are no saved transactions. */
    if (transactionCount == 0) {
        addTransaction(currentDate, 0, "Monthly Salary", 50000, 1);
        addTransaction(currentDate, 0, "Lunch with friends", 1500, 0);
        addTransaction(currentDate, 1, "Uber", 800, 0);
        addTransaction(currentDate, 3, "Shoes", 4000, 0);

        printf("Negative amount test (expect 0): %d\n",
               addTransaction(currentDate, 0, "Test", -5, 0));
        printf("Invalid date test (expect 0): %d\n",
               addTransaction(invalidDate, 0, "Test", 5, 0));

        /* Set the sample payment date a few days from today. */
        if (paymentDate.day > 25) {
            paymentDate.day = 25;
        } else {
            paymentDate.day = paymentDate.day + 3;
        }

        addPayment("College Fee", 8000, paymentDate, 1);

        monthlyBudget = 20000;
        categoryBudget[0] = 1000;
        categoryBudget[3] = 5000;
        reserveAmount = 5000;

        strcpy(savingsGoal.name, "New Laptop");
        savingsGoal.targetAmount = 150000;
        savingsGoal.savedAmount = 55000;

        saveData();
    }

    printf("Income: %.0f\n", totalIncome());
    printf("Expenses: %.0f\n", totalExpenses());
    printf("Balance: %.0f\n", balance());
    printf("Savings goal: %.1f%%\n", goalPercent());
    printf("Health score: %d\n", healthScore());
    printf("Can afford Rs. 20000? %d\n", canAfford(20000));
    printf("Can afford Rs. 60000? %d\n", canAfford(60000));
    printf("Months needed when saving Rs. 19000 each month: %.1f\n",
           monthsToGoal(19000));

    makeAlerts();

    printf("\nAlerts:\n");
    for (i = 0; i < alertCount; i++) {
        printf("- %s\n", alerts[i]);
    }

    if (transactionCount > 0) {
        printf("\nFirst transaction note: %s\n", transactions[0].note);
    }

    if (paymentCount > 0) {
        printf("First payment: %s\n", paymentList[0].name);
        printf("Payment due date before marking paid: %d/%d\n",
               paymentList[0].dueDate.day, paymentList[0].dueDate.month);

        markPaid(0);

        if (paymentCount > 0) {
            printf("Payment due date after marking paid: %d/%d\n",
                   paymentList[0].dueDate.day, paymentList[0].dueDate.month);
        }

        printf("Transactions after marking paid: %d\n", transactionCount);
    }

    saveData();
    return 0;
}
