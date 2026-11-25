/* smartspend.h
   Shared structures, variables and function names for SmartSpend.
*/
#ifndef SMARTSPEND_H
#define SMARTSPEND_H

#define MAX_TRANSACTIONS 1000
#define MAX_PAYMENTS 50
#define NUM_CATEGORIES 7
#define MAX_ALERTS 20

/* A date has a day, month and year. */
typedef struct {
    int day;
    int month;
    int year;
} Date;

/* One income or expense record. isIncome is 1 for income and 0 for expense. */
typedef struct {
    Date date;
    int category;
    char note[50];
    float amount;
    int isIncome;
} Transaction;

/* One payment that the user needs to pay. */
typedef struct {
    char name[30];
    float amount;
    Date dueDate;
    int repeatsMonthly;
} Payment;

/* The savings goal entered by the user. */
typedef struct {
    char name[30];
    float targetAmount;
    float savedAmount;
} SavingsGoal;


/* Variables shared by the source files. */
extern char categoryNames[NUM_CATEGORIES][20];

extern Transaction transactions[MAX_TRANSACTIONS];
extern int transactionCount;

extern Payment paymentList[MAX_PAYMENTS];
extern int paymentCount;

extern float monthlyBudget;
extern float reserveAmount;
extern float categoryBudget[NUM_CATEGORIES];

extern SavingsGoal savingsGoal;

extern char alerts[MAX_ALERTS][100];
extern int alertCount;


/* Functions from data.c */
Date today(void);
Date parseDate(char text[]);
int isValidDate(Date d);
int isLeapYear(int year);
int getDaysInMonth(int month, int year);

int addTransaction(Date d, int category, char note[],
                   float amount, int isIncome);
int addPayment(char name[], float amount, Date dueDate, int repeatsMonthly);

void deletePayment(int index);
void sortPayments(void);
void saveData(void);
void loadData(void);


/* Functions from finance.c */
float totalIncome(void);
float totalExpenses(void);
float balance(void);

float spentInMonth(int month, int year);
float spentInCategory(int category, int month, int year);

int daysUntil(Date d);
void rollRecurring(void);
void markPaid(int index);
float upcomingPayments(int withinDays);

float goalPercent(void);
float monthsToGoal(float perMonth);

void makeAlerts(void);
int healthScore(void);
int canAfford(float price);

#endif
