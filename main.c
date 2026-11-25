/* main.c
   This file displays the SmartSpend window and its screens.
   data.c saves and loads information.
   finance.c performs the financial calculations.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "smartspend.h"

#define CONTENT_X 220                 //where the main area of every screen starts //

// ---------- global variables ---------- //

int currentScreen = 0;                  /* 0 Dashboard, 1 Add, 2 History, 3 Budget, 4 Payments, 5 Goal, 6 Advisor */
int activeTextBox = -1;              /* the text box the user is typing in (-1 = none) */
char statusMessage[100] = "";          /* message shown under a Save button */
int messageIsSuccess = 0;             /* 1 = green message, 0 = red message */

char transactionAmountText[16], transactionNoteText[40], transactionDateText[16];            //Add screen //
int chosenCategory = 0, transactionIsIncome = 0;
char historySearchText[30] = "";                                   /* History screen */
int currentHistoryPage = 0;
char monthlyBudgetText[16], categoryBudgetText[NUM_CATEGORIES][16]; /* Budget screen */
char paymentNameText[30], paymentAmountText[16], paymentDateText[16];   /* Payments screen */
int paymentRepeatsMonthly = 1;
char savingsGoalNameText[30], savingsTargetText[16], savingsSavedText[16], emergencyReserveText[16];   /* Goal screen */
char purchasePriceText[16] = "", monthlySavingText[16] = "", targetMonthsText[16] = "";                 /* Advisor screen */

/* ================= small helper functions ================= */

/* "12.5" -> 12.5.   Returns -1 if the text is empty or is not a number. */
float readAmount(char text[]) {
    int i;
    int decimalPoints = 0;

    if (text[0] == '\0') {
        return -1;
    }

    /* Check that the user typed digits and no more than one decimal point. */
    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] == '.') {
            decimalPoints++;
        } else if (text[i] < '0' || text[i] > '9') {
            return -1;
        }
    }

    if (decimalPoints > 1) {
        return -1;
    }

    return (float)atof(text);
}

// 1 if "text" contains "word" (capital letters do not matter) //
int textMatches(const char text[], const char word[]) {
    char a[100], b[100];
    int i;
    for (i = 0; text[i] != '\0'; i++) a[i] = (char)tolower((unsigned char)text[i]);
    a[i] = '\0';
    for (i = 0; word[i] != '\0'; i++) b[i] = (char)tolower((unsigned char)word[i]);
    b[i] = '\0';
    return strstr(a, b) != NULL;
}

void setStatusMessage(int good, const char text[]) {          // sets the message under a Save button //
    strcpy(statusMessage, text);
    messageIsSuccess = good;
}

void drawStatusMessage(int x, int y) {
    if (messageIsSuccess == 1) DrawText(statusMessage, x, y, 20, DARKGREEN);
    else DrawText(statusMessage, x, y, 20, MAROON);
}

/* text for a payment: "due in 3 days", "due TODAY" or "overdue by 2 days" */
const char *paymentDateStatus(int days) 
{
    if (days < 0) return TextFormat("overdue by %d days", -days);
    if (days == 0) return "due TODAY";
    return TextFormat("due in %d days", days);
}

/* a text box. Every box needs its own id number. */
void drawTextBox(int id, int x, int y, int w, char text[], int size)
 {
    Rectangle r = { x, y, w, 34 };
    if (GuiTextBox(r, text, size, activeTextBox == id)) {
        if (activeTextBox == id) activeTextBox = -1;     /* clicked again: stop typing */
        else activeTextBox = id;                     /* clicked: start typing here */
    }
}


void drawInputField(int id, int x, int y, int w, const char label[], char text[], int size) {
    DrawText(label, x, y, 18, GRAY);
    drawTextBox(id, x, y + 23, w, text, size);
}

//a button: returns 1 when clicked //
int drawButton(int x, int y, int w, int h, const char text[]) {
    Rectangle r = { x, y, w, h };
    return GuiButton(r, text);
}

/* a thick coloured border, shows which button is selected */
void drawSelectionBorder(int x, int y, int w, int h, Color c) 
{
    for (int i = 0; i < 3; i++) 
    DrawRectangleLines(x + i, y + i, w - 2 * i, h - 2 * i, c);
}

/* white boxes: "card" has a small title and a big value, "panel" only a title */
void drawSummaryCard(int x, int y, const char title[], const char value[], Color c) 
{
    DrawRectangle(x, y, 175, 90, WHITE);
    DrawText(title, x + 12, y + 10, 18, GRAY);
    DrawText(value, x + 12, y + 42, 28, c);
}

void drawPanel(int x, int y, int w, int h, const char title[]) {
    DrawRectangle(x, y, w, h, WHITE);
    DrawText(title, x + 14, y + 10, 20, DARKGRAY);
}

/* a bar showing how much of a budget is used (green, orange at 80%, red at 100%) */
void drawBudgetBar(int x, int y, float spent, float limit)
 {
    DrawRectangle(x, y, 240, 18, LIGHTGRAY);
    if (limit <= 0) {
        DrawText(TextFormat("%.0f spent (no limit)", spent), x + 250, y, 16, GRAY);
        return;
    }
    float ratio = spent / limit;
    Color c = GREEN;
    if (ratio >= 0.8) c = ORANGE;
    if (ratio >= 1.0) c = MAROON;
    if (ratio > 1) ratio = 1;
    DrawRectangle(x, y, (int)(240 * ratio), 18, c);
    DrawText(TextFormat("%.0f / %.0f", spent, limit), x + 250, y, 16, DARKGRAY);
}

/* ---- fill the text boxes with today's date / saved values when a screen opens ---- */

void clearTransactionForm(void)
 {
    Date t = today();
    transactionAmountText[0] = '\0';
    transactionNoteText[0] = '\0';
    sprintf(transactionDateText, "%02d/%02d/%04d", t.day, t.month, t.year);
}

void clearPaymentForm(void) 
{
    Date t = today();
    paymentNameText[0] = '\0';
    paymentAmountText[0] = '\0';
    sprintf(paymentDateText, "%02d/%02d/%04d", t.day, t.month, t.year);
}

void fillBudgetForm(void) 
{
    sprintf(monthlyBudgetText, "%.0f", monthlyBudget);
    for (int i = 0; i < NUM_CATEGORIES; i++) sprintf(categoryBudgetText[i], "%.0f", categoryBudget[i]);
}

void fillSavingsForm(void) {
    if (strcmp(savingsGoal.name, "None") == 0) savingsGoalNameText[0] = '\0';
    else strcpy(savingsGoalNameText, savingsGoal.name);
    sprintf(savingsTargetText, "%.0f", savingsGoal.targetAmount);
    sprintf(savingsSavedText, "%.0f", savingsGoal.savedAmount);
    sprintf(emergencyReserveText, "%.0f", reserveAmount);
}

/* ================= sidebar ================= */

void drawMenu(void) 
{
    char names[7][20] = { "Dashboard", "Add Transaction", "History", "Budget", "Payments", "Goal & Reserve", "Advisor" };

    DrawRectangle(0, 0, 190, 650, (Color){30, 41, 59, 255});
    DrawText("SmartSpend", 20, 24, 26, RAYWHITE);

    for (int i = 0; i < 7; i++) 
    {
        int y = 85 + i * 54;
        if (drawButton(15, y, 160, 42, names[i])) {
            currentScreen = i;
            activeTextBox = -1;
            statusMessage[0] = '\0';
            currentHistoryPage = 0;
            if (i == 3) fillBudgetForm();
            if (i == 4) clearPaymentForm();
            if (i == 5) fillSavingsForm();
        }
        if (currentScreen == i) drawSelectionBorder(15, y, 160, 42, SKYBLUE);
    }
}

//================= screen 0: dashboard ================= //

void drawDashboard(void) {
    Date t = today();
    int score = healthScore();
    Color scoreColor = MAROON;
    if (score >= 50) scoreColor = ORANGE;
    if (score >= 75) scoreColor = DARKGREEN;

    DrawText("Dashboard", CONTENT_X, 20, 30, DARKGRAY);
    DrawText(TextFormat("Today: %02d/%02d/%d", t.day, t.month, t.year), CONTENT_X + 560, 28, 20, GRAY);

    drawSummaryCard(CONTENT_X,       70, "Total Income",   TextFormat("Rs. %.0f", totalIncome()),   DARKGREEN);
    drawSummaryCard(CONTENT_X + 190, 70, "Total Expenses", TextFormat("Rs. %.0f", totalExpenses()), MAROON);
    drawSummaryCard(CONTENT_X + 380, 70, "Balance",        TextFormat("Rs. %.0f", balance()),       DARKBLUE);
    drawSummaryCard(CONTENT_X + 570, 70, "Health Score",   TextFormat("%d%%", score),               scoreColor);

    drawPanel(CONTENT_X, 175, 745, 90, "Savings Goal");
    if (savingsGoal.targetAmount > 0) {
        DrawText(TextFormat("%s:  Rs. %.0f of Rs. %.0f  (%.0f%%)", savingsGoal.name, savingsGoal.savedAmount, savingsGoal.targetAmount, goalPercent()), CONTENT_X + 14, 205, 18, DARKGRAY);
        DrawRectangle(CONTENT_X + 14, 235, 715, 16, LIGHTGRAY);
        DrawRectangle(CONTENT_X + 14, 235, (int)(715 * goalPercent() / 100), 16, GREEN);
    } else {
        DrawText("No savings goal set yet.", CONTENT_X + 14, 215, 18, GRAY);
    }

    drawPanel(CONTENT_X, 280, 745, 160, "Alerts");
    makeAlerts();
    if (alertCount == 0) DrawText("No alerts. Everything looks fine!", CONTENT_X + 14, 315, 18, DARKGREEN);
    for (int i = 0; i < alertCount && i < 5; i++) {
        DrawText(TextFormat("- %s", alerts[i]), CONTENT_X + 14, 315 + i * 24, 17, DARKGRAY);
    }

    drawPanel(CONTENT_X, 455, 745, 175, "Upcoming Payments");
    if (paymentCount == 0) DrawText("No payments added yet.", CONTENT_X + 14, 490, 18, GRAY);
    for (int i = 0; i < paymentCount && i < 5; i++) {
        DrawText(TextFormat("%s  -  Rs. %.0f  -  %s", paymentList[i].name, paymentList[i].amount, paymentDateStatus(daysUntil(paymentList[i].dueDate))),
                 CONTENT_X + 14, 490 + i * 26, 18, DARKGRAY);
    }
}

//================= screen 1: add transaction =================//

void saveTransactionForm(void) {
    float amount = readAmount(transactionAmountText);
    Date d = parseDate(transactionDateText);

    if (amount <= 0) setStatusMessage(0, "Enter an amount greater than 0.");
    else if (!isValidDate(d)) setStatusMessage(0, "Date must be a real date like 25/11/2026.");
    else if (addTransaction(d, chosenCategory, transactionNoteText, amount, transactionIsIncome)) {
        saveData();
        if (transactionIsIncome == 1) setStatusMessage(1, "Income saved!");
        else setStatusMessage(1, "Expense saved!");
        clearTransactionForm();
    } else setStatusMessage(0, "Could not save (list full?).");
}

void drawAddTransaction(void) {
    DrawText("Add Transaction", CONTENT_X, 20, 30, DARKGRAY);

    DrawText("Type", CONTENT_X, 75, 18, GRAY);
    if (drawButton(CONTENT_X, 98, 120, 36, "Expense")) transactionIsIncome = 0;
    if (drawButton(CONTENT_X + 135, 98, 120, 36, "Income")) transactionIsIncome = 1;
    if (transactionIsIncome == 0) drawSelectionBorder(CONTENT_X, 98, 120, 36, MAROON);
    else drawSelectionBorder(CONTENT_X + 135, 98, 120, 36, DARKGREEN);

    drawInputField(1, CONTENT_X, 150, 250, "Amount (Rs.)", transactionAmountText, 16);
    drawInputField(2, CONTENT_X, 220, 250, "Date (DD/MM/YYYY)", transactionDateText, 16);
    drawInputField(3, CONTENT_X, 290, 400, "Note (optional)", transactionNoteText, 40);

    DrawText("Category", CONTENT_X, 362, 18, GRAY);
    for (int c = 0; c < NUM_CATEGORIES; c++) {
        int x = CONTENT_X + (c % 4) * 135;
        int y = 385 + (c / 4) * 44;
        if (drawButton(x, y, 125, 36, categoryNames[c])) chosenCategory = c;
        if (c == chosenCategory) drawSelectionBorder(x, y, 125, 36, DARKBLUE);
    }

    if (drawButton(CONTENT_X, 490, 170, 46, "SAVE")) saveTransactionForm();
    drawStatusMessage(CONTENT_X, 552);
}

/* ================= screen 2: history ================= */

void drawTransactionHistory(void) {
    int shown[MAX_TRANSACTIONS];          /* positions of the transactions that match the search */
    int shownCount = 0;

    DrawText("Transaction History", CONTENT_X, 20, 30, DARKGRAY);
    DrawText("Search by note or category:", CONTENT_X, 72, 18, GRAY);
    drawTextBox(4, CONTENT_X + 250, 66, 300, historySearchText, 30);

    /* newest first: keep only the transactions that match the search */
    for (int i = transactionCount - 1; i >= 0; i--) {
        if (textMatches(transactions[i].note, historySearchText) || textMatches(categoryNames[transactions[i].category], historySearchText)) {
            shown[shownCount] = i;
            shownCount++;
        }
    }

    int totalPages = (shownCount + 11) / 12;         /* 12 rows on each page */
    if (totalPages < 1) totalPages = 1;
    if (currentHistoryPage >= totalPages) currentHistoryPage = totalPages - 1;
    if (currentHistoryPage < 0) currentHistoryPage = 0;

    DrawRectangle(CONTENT_X, 115, 745, 30, (Color){226, 232, 240, 255});
    DrawText("Date", CONTENT_X + 10, 121, 18, DARKGRAY);
    DrawText("Category", CONTENT_X + 130, 121, 18, DARKGRAY);
    DrawText("Note", CONTENT_X + 290, 121, 18, DARKGRAY);
    DrawText("Amount", CONTENT_X + 620, 121, 18, DARKGRAY);

    for (int row = 0; row < 12; row++) {
        int k = currentHistoryPage * 12 + row;
        if (k >= shownCount) break;
        Transaction t = transactions[shown[k]];
        int y = 150 + row * 36;

        DrawText(TextFormat("%02d/%02d/%d", t.date.day, t.date.month, t.date.year), CONTENT_X + 10, y + 8, 18, DARKGRAY);
        DrawText(categoryNames[t.category], CONTENT_X + 130, y + 8, 18, DARKGRAY);

        
        DrawText(t.note, CONTENT_X + 290, y + 8, 18, DARKGRAY);
        if (t.isIncome == 1) DrawText(TextFormat("+%.0f", t.amount), CONTENT_X + 620, y + 8, 18, DARKGREEN);
        else DrawText(TextFormat("-%.0f", t.amount), CONTENT_X + 620, y + 8, 18, MAROON);
    }


    if (shownCount == 0) DrawText("No transactions found.", CONTENT_X + 10, 160, 18, GRAY);

    if (drawButton(CONTENT_X, 598, 100, 34, "< Prev")) currentHistoryPage--;
    if (drawButton(CONTENT_X + 115, 598, 100, 34, "Next >")) currentHistoryPage++;
    DrawText(TextFormat("Page %d of %d   (%d transactions)", currentHistoryPage + 1, totalPages, shownCount), CONTENT_X + 235, 606, 18, GRAY);
}

/* ================= screen 3: budget ================= */

void saveBudgetForm(void) {
    float monthly = readAmount(monthlyBudgetText);
    float category[NUM_CATEGORIES];
    int allOk = (monthly >= 0);

    for (int i = 0; i < NUM_CATEGORIES; i++) {
        category[i] = readAmount(categoryBudgetText[i]);
        if (category[i] < 0) allOk = 0;
    }
    if (allOk == 0) {
        setStatusMessage(0, "Every box must be a number (use 0 for no limit).");
        return;
    }
    monthlyBudget = monthly;
    for (int i = 0; i < NUM_CATEGORIES; i++) categoryBudget[i] = category[i];
    saveData();
    setStatusMessage(1, "Budgets saved!");
}

void drawBudgetScreen(void) {
    Date t = today();

    DrawText("Monthly Budget Planner", CONTENT_X, 20, 30, DARKGRAY);
    DrawText(TextFormat("Month: %02d/%d", t.month, t.year), CONTENT_X + 590, 28, 20, GRAY);

    DrawText("Monthly budget (Rs.)", CONTENT_X, 86, 18, DARKGRAY);
    drawTextBox(10, CONTENT_X + 190, 78, 120, monthlyBudgetText, 16);
    drawBudgetBar(CONTENT_X + 320, 86, spentInMonth(t.month, t.year), monthlyBudget);

    for (int c = 0; c < NUM_CATEGORIES; c++) {
        int y = 140 + c * 44;
        DrawText(categoryNames[c], CONTENT_X, y + 8, 18, DARKGRAY);
        drawTextBox(11 + c, CONTENT_X + 190, y, 120, categoryBudgetText[c], 16);
        drawBudgetBar(CONTENT_X + 320, y + 8, spentInCategory(c, t.month, t.year), categoryBudget[c]);
    }

    if (drawButton(CONTENT_X, 462, 190, 44, "SAVE BUDGETS")) saveBudgetForm();
    drawStatusMessage(CONTENT_X, 522);
    DrawText("Bars turn orange at 80% and red when the budget is exceeded. Use 0 for no limit.", CONTENT_X, 580, 16, GRAY);
}

/* ================= screen 4: payments ================= */

void savePaymentForm(void) {
    float amount = readAmount(paymentAmountText);
    Date d = parseDate(paymentDateText);

    if (paymentNameText[0] == '\0') setStatusMessage(0, "Enter a payment name.");
    else if (amount <= 0) setStatusMessage(0, "Enter an amount greater than 0.");
    else if (!isValidDate(d)) setStatusMessage(0, "Date must be a real date like 25/11/2026.");
    else if (addPayment(paymentNameText, amount, d, paymentRepeatsMonthly)) {
        sortPayments();
        saveData();
        setStatusMessage(1, "Payment added!");
        clearPaymentForm();
    } else setStatusMessage(0, "Could not add payment (list full?).");
}

void drawPaymentScreen(void) {
    int paidRow = -1, deleteRow = -1;        /* which row's button was clicked */

    DrawText("Upcoming Payments", CONTENT_X, 20, 30, DARKGRAY);
    drawInputField(20, CONTENT_X, 72, 230, "Name", paymentNameText, 30);
    drawInputField(21, CONTENT_X + 245, 72, 130, "Amount (Rs.)", paymentAmountText, 16);
    drawInputField(22, CONTENT_X + 390, 72, 170, "Due date (DD/MM/YYYY)", paymentDateText, 16);

    if (paymentRepeatsMonthly == 1) {
        if (drawButton(CONTENT_X + 580, 95, 165, 34, "Repeats monthly: YES")) paymentRepeatsMonthly = 0;
    } else {
        if (drawButton(CONTENT_X + 580, 95, 165, 34, "Repeats monthly: NO")) paymentRepeatsMonthly = 1;
    }
    if (drawButton(CONTENT_X, 145, 170, 40, "ADD PAYMENT")) savePaymentForm();
    drawStatusMessage(CONTENT_X + 190, 155);

    drawPanel(CONTENT_X, 200, 745, 430, "Your payments (earliest first)");
    if (paymentCount == 0) DrawText("No payments added yet.", CONTENT_X + 14, 245, 18, GRAY);

    for (int i = 0; i < paymentCount && i < 8; i++) {
        int y = 240 + i * 46;
        Payment p = paymentList[i];
        DrawText(p.name, CONTENT_X + 14, y + 8, 18, DARKGRAY);
        DrawText(TextFormat("Rs. %.0f", p.amount), CONTENT_X + 170, y + 8, 18, DARKGRAY);
        DrawText(TextFormat("%02d/%02d/%d", p.dueDate.day, p.dueDate.month, p.dueDate.year), CONTENT_X + 280, y + 8, 18, DARKGRAY);
        DrawText(paymentDateStatus(daysUntil(p.dueDate)), CONTENT_X + 395, y + 8, 18, DARKGRAY);
        if (drawButton(CONTENT_X + 575, y, 75, 32, "Paid")) paidRow = i;
        if (drawButton(CONTENT_X + 660, y, 75, 32, "Delete")) deleteRow = i;
    }

    /* the list is changed AFTER the loop, so the loop is not disturbed */
    if (paidRow != -1) {
        markPaid(paidRow);
        saveData();
        setStatusMessage(1, "Paid! Saved as a Bills expense.");
    }
    if (deleteRow != -1) {
        deletePayment(deleteRow);
        saveData();
        setStatusMessage(1, "Payment deleted.");
    }
}

/* ================= screen 5: goal and reserve ================= */

void saveSavingsForm(void) {
    float targetAmount = readAmount(savingsTargetText);
    float savedAmount = readAmount(savingsSavedText);
    float reserve = readAmount(emergencyReserveText);

    if (savingsGoalNameText[0] == '\0') setStatusMessage(0, "Enter a goal name (for example New Laptop).");
    else if (targetAmount <= 0) setStatusMessage(0, "Target must be a number greater than 0.");
    else if (savedAmount < 0 || reserve < 0) setStatusMessage(0, "Saved amount and reserve must be numbers.");
    else {
        strcpy(savingsGoal.name, savingsGoalNameText);
        savingsGoal.targetAmount = targetAmount;
        savingsGoal.savedAmount = savedAmount;
        reserveAmount = reserve;
        saveData();
        setStatusMessage(1, "Saved!");
    }
}

void drawSavingsScreen(void) {
    DrawText("Savings Goal & Reserve", CONTENT_X, 20, 30, DARKGRAY);
    drawInputField(30, CONTENT_X, 75, 300, "Goal name", savingsGoalNameText, 30);
    drawInputField(31, CONTENT_X, 150, 200, "Target amount (Rs.)", savingsTargetText, 16);
    drawInputField(32, CONTENT_X, 225, 200, "Saved so far (Rs.)", savingsSavedText, 16);
    drawInputField(33, CONTENT_X, 310, 200, "Emergency reserve amount (Rs.)", emergencyReserveText, 16);

    if (drawButton(CONTENT_X, 395, 170, 44, "SAVE")) saveSavingsForm();
    drawStatusMessage(CONTENT_X, 455);

    drawPanel(CONTENT_X + 400, 75, 345, 270, "Current status");
    if (savingsGoal.targetAmount > 0) {
        float remaining = savingsGoal.targetAmount - savingsGoal.savedAmount;
        if (remaining < 0) remaining = 0;
        DrawText(savingsGoal.name, CONTENT_X + 414, 115, 20, DARKBLUE);
        DrawRectangle(CONTENT_X + 414, 150, 315, 18, LIGHTGRAY);
        DrawRectangle(CONTENT_X + 414, 150, (int)(315 * goalPercent() / 100), 18, GREEN);
        DrawText(TextFormat("%.0f%% complete", goalPercent()), CONTENT_X + 414, 176, 18, DARKGRAY);
        DrawText(TextFormat("Remaining: Rs. %.0f", remaining), CONTENT_X + 414, 204, 18, DARKGRAY);
    } else {
        DrawText("No goal set yet.", CONTENT_X + 414, 120, 18, GRAY);
    }
    DrawText(TextFormat("Balance: Rs. %.0f", balance()), CONTENT_X + 414, 250, 18, DARKGRAY);
    DrawText(TextFormat("Reserve: Rs. %.0f", reserveAmount), CONTENT_X + 414, 276, 18, DARKGRAY);
    if (balance() >= reserveAmount) DrawText("Reserve is covered.", CONTENT_X + 414, 304, 18, DARKGREEN);
    else DrawText("Balance is BELOW your reserve!", CONTENT_X + 414, 304, 18, MAROON);
}

/* ================= screen 6: advisor =================
   The answers here update LIVE while you type, so there is no Check button. */

void drawAdvisorScreen(void) {
    Date t = today();
    DrawText("Advisor", CONTENT_X, 20, 30, DARKGRAY);

    /* --- can I afford this? --- */
    drawPanel(CONTENT_X, 65, 745, 150, "Can I afford this purchase?");
    drawInputField(40, CONTENT_X + 14, 100, 150, "Price (Rs.)", purchasePriceText, 16);
    float price = readAmount(purchasePriceText);
    if (price > 0) {
        float left = balance() - upcomingPayments(30) - reserveAmount - price;
        DrawText(TextFormat("Left after bills + reserve + price: Rs. %.0f", left), CONTENT_X + 190, 110, 18, DARKGRAY);
        if (canAfford(price)) DrawText("YES - you can afford this purchase.", CONTENT_X + 190, 140, 20, DARKGREEN);
        else DrawText(TextFormat("NO - you would be Rs. %.0f short. Better wait or save first.", -left), CONTENT_X + 190, 140, 20, MAROON);
    } else {
        DrawText("Type a price to see the answer.", CONTENT_X + 190, 125, 18, GRAY);
    }

    /* --- savings goal planner --- */
    drawPanel(CONTENT_X, 225, 745, 190, "Savings goal planner");
    drawInputField(41, CONTENT_X + 14, 260, 170, "Monthly saving (Rs.)", monthlySavingText, 16);
    drawInputField(42, CONTENT_X + 200, 260, 170, "Months wanted", targetMonthsText, 16);
    float perMonth = readAmount(monthlySavingText);
    float wanted = readAmount(targetMonthsText);
    float remaining = savingsGoal.targetAmount - savingsGoal.savedAmount;

    if (savingsGoal.targetAmount <= 0) {
        DrawText("Set a savings goal first (Goal & Reserve screen).", CONTENT_X + 14, 335, 18, MAROON);
    } else if (remaining <= 0) {
        DrawText("Your savings goal is already reached!", CONTENT_X + 14, 335, 18, DARKGREEN);
    } else if (perMonth > 0) {
        DrawText(TextFormat("Still needed: Rs. %.0f.  At Rs. %.0f per month you reach it in %.1f months.", remaining, perMonth, monthsToGoal(perMonth)), CONTENT_X + 14, 335, 17, DARKGRAY);
        if (wanted > 0 && perMonth >= remaining / wanted) {
            DrawText(TextFormat("To finish in %.0f months you need Rs. %.0f per month: your plan IS enough.", wanted, remaining / wanted), CONTENT_X + 14, 365, 17, DARKGREEN);
        } else if (wanted > 0) {
            DrawText(TextFormat("To finish in %.0f months you need Rs. %.0f per month: NOT enough.", wanted, remaining / wanted), CONTENT_X + 14, 365, 17, MAROON);
        }
    } else {
        DrawText("Type a monthly saving amount to see the plan.", CONTENT_X + 14, 335, 18, GRAY);
    }

    /* --- this month compared with last month --- */
    drawPanel(CONTENT_X, 425, 745, 205, "Spending pattern: this month vs last month");
    int lastMonth = t.month - 1, lastYear = t.year;
    if (lastMonth < 1) {
        lastMonth = 12;
        lastYear = lastYear - 1;
    }
    float thisTotal = spentInMonth(t.month, t.year);
    float lastTotal = spentInMonth(lastMonth, lastYear);
    DrawText(TextFormat("This month: Rs. %.0f        Last month: Rs. %.0f", thisTotal, lastTotal), CONTENT_X + 14, 462, 18, DARKGRAY);

    if (lastTotal <= 0) DrawText("No spending recorded last month to compare.", CONTENT_X + 14, 490, 18, GRAY);
    else if (thisTotal > lastTotal) DrawText(TextFormat("Spending went UP by %.0f%%", (thisTotal - lastTotal) / lastTotal * 100), CONTENT_X + 14, 490, 18, MAROON);
    else DrawText(TextFormat("Spending went down or stayed the same (%.0f%%)", (thisTotal - lastTotal) / lastTotal * 100), CONTENT_X + 14, 490, 18, DARKGREEN);

    char upList[120] = "";            /* names of the categories that went up */
    char topName[20] = "-";
    float topAmount = 0;
    for (int c = 0; c < NUM_CATEGORIES; c++) {
        float a = spentInCategory(c, t.month, t.year);
        if (a > spentInCategory(c, lastMonth, lastYear)) {
            if (upList[0] != '\0') strcat(upList, ", ");
            strcat(upList, categoryNames[c]);
        }
        if (a > topAmount) {
            topAmount = a;
            strcpy(topName, categoryNames[c]);
        }
    }
    DrawText(TextFormat("Biggest category this month: %s (Rs. %.0f)", topName, topAmount), CONTENT_X + 14, 520, 18, DARKGRAY);
    if (upList[0] != '\0') DrawText(TextFormat("Spending more than last month in: %s", upList), CONTENT_X + 14, 550, 18, DARKGRAY);
    else DrawText("Spending more than last month in: none", CONTENT_X + 14, 550, 18, DARKGRAY);
}

/* ================= main ================= */

int main(void) {
    loadData();                  /* read the saved data from the text files */
    rollRecurring();             /* late repeating payments move to next month */
    saveData();

    InitWindow(1000, 650, "SmartSpend");
    SetExitKey(KEY_NULL);        /* ESC must not close the window while typing */
    SetTargetFPS(60);
    clearTransactionForm();

    while (!WindowShouldClose()) {           /* the main loop */
        BeginDrawing();
        ClearBackground((Color){241, 245, 249, 255});

        drawMenu();
        if (currentScreen == 0) drawDashboard();
        if (currentScreen == 1) drawAddTransaction();
        if (currentScreen == 2) drawTransactionHistory();
        if (currentScreen == 3) drawBudgetScreen();
        if (currentScreen == 4) drawPaymentScreen();
        if (currentScreen == 5) drawSavingsScreen();
        if (currentScreen == 6) drawAdvisorScreen();

        EndDrawing();
    }

    saveData();
    CloseWindow();
    return 0;
}
