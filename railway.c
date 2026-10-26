#include <stdio.h>
#include <stdlib.h>

struct station {
    char ch;
    int is_end;
    struct station *child;
    struct station *sibling;
};
struct station *root = NULL;

struct train {
    int no;
    char name[20], from[15], to[15];
    int seats, lower;
    float fare;
};

struct train trains[3] = {
    {101, "Rajdhani", "Mumbai",  "Delhi",     6, 2, 2500},
    {102, "Shatabdi", "Mumbai",  "Ahmedabad", 4, 1, 900},
    {103, "Duronto",  "Kolkata", "Mumbai",    5, 2, 2200}
};

struct passenger {
    int pnr, train_no, age, lower_berth;
    char name[30];
    struct passenger *next;
};
struct passenger *head = NULL;
int next_pnr = 1001;

struct waiting {
    int pnr, train_no, age;
    struct waiting *next;
};
struct waiting *start = NULL;

char lowerCase(char c) {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

void flushInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int readInt(const char *msg, int *x) {
    printf("%s", msg);
    if (scanf("%d", x) != 1) {
        flushInput();
        printf("Invalid input!\n");
        return 0;
    }
    return 1;
}

struct station *newStation(char ch) {
    struct station *n = (struct station *)malloc(sizeof(struct station));
    n->ch = ch;
    n->is_end = 0;
    n->child = n->sibling = NULL;
    return n;
}

void insertStation(const char *name) {
    struct station *cur, *p, *prev;
    int i;
    if (root == NULL)
        root = newStation('*');
    cur = root;
    for (i = 0; name[i] != '\0'; i++) {
        prev = NULL;
        p = cur->child;
        while (p != NULL && p->ch != lowerCase(name[i])) {
            prev = p;
            p = p->sibling;
        }
        if (p == NULL) {
            p = newStation(lowerCase(name[i]));
            if (prev == NULL)
                cur->child = p;
            else
                prev->sibling = p;
        }
        cur = p;
    }
    cur->is_end = 1;
}

int stationExists(const char *name) {
    struct station *cur = root, *p;
    int i;
    if (root == NULL)
        return 0;
    for (i = 0; name[i] != '\0'; i++) {
        p = cur->child;
        while (p != NULL && p->ch != lowerCase(name[i]))
            p = p->sibling;
        if (p == NULL)
            return 0;
        cur = p;
    }
    return cur->is_end;
}

int sameName(const char *a, const char *b) {
    while (*a && lowerCase(*a) == lowerCase(*b)) {
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

struct train *findTrain(int no) {
    int i;
    for (i = 0; i < 3; i++)
        if (trains[i].no == no)
            return &trains[i];
    return NULL;
}

struct passenger *findPassenger(int pnr) {
    struct passenger *p = head;
    while (p != NULL && p->pnr != pnr)
        p = p->next;
    return p;
}

void searchStation() {
    char name[30];
    int i;
    printf("Enter station name: ");
    if (scanf("%29s", name) != 1 || !stationExists(name)) {
        printf("Station not found.\n");
        return;
    }
    printf("Station '%s' found! Trains:\n", name);
    for (i = 0; i < 3; i++)
        if (sameName(trains[i].from, name) ||
            sameName(trains[i].to, name))
            printf("  %d %s (%s -> %s) Fare %.0f\n",
                   trains[i].no,
                   trains[i].name,
                   trains[i].from,
                   trains[i].to,
                   trains[i].fare);
}

void enqueueSenior(int pnr, int train_no, int age) {
    struct waiting *t;
    struct waiting *n =
        (struct waiting *)malloc(sizeof(struct waiting));
    if (n == NULL) {
        printf("Failed to join the queue!\n");
        return;
    }
    n->pnr = pnr;
    n->train_no = train_no;
    n->age = age;
    if (start == NULL || age > start->age) {
        n->next = start;
        start = n;
        return;
    }
    t = start;
    while (t->next != NULL && t->next->age >= age)
        t = t->next;
    n->next = t->next;
    t->next = n;
}

void removeFromQueue(int pnr) {
    struct waiting *t = start, *prev = NULL;
    while (t != NULL && t->pnr != pnr) {
        prev = t;
        t = t->next;
    }
    if (t == NULL)
        return;
    if (prev == NULL)
        start = t->next;
    else
        prev->next = t->next;
    free(t);
}

void showQueue() {
    struct waiting *t = start;
    int pos = 1;
    if (t == NULL) {
        printf("The lower berth queue is empty.\n");
        return;
    }
    printf("Pos PNR   Train Age\n");
    while (t != NULL) {
        printf("%-4d %-5d %-6d %d\n",
               pos++, t->pnr, t->train_no, t->age);
        t = t->next;
    }
}

void bookTicket() {
    int no, age;
    struct train *t;
    struct passenger *p;

    if (!readInt("Enter train number = ", &no))
        return;
    t = findTrain(no);
    if (t == NULL || t->seats == 0) {
        printf("Train not found or full.\n");
        return;
    }
    p = (struct passenger *)malloc(sizeof(struct passenger));
    if (p == NULL) {
        printf("Booking failed, try again.\n");
        return;
    }
    printf("Enter passenger name = ");
    scanf("%29s", p->name);
    if (!readInt("Enter age = ", &age) || age < 1 || age > 120) {
        printf("Age must be between 1 and 120.\n");
        free(p);
        return;
    }

    if (age < 60 && t->seats == t->lower) {
        printf("Only lower berths are left, reserved for senior citizens.\n");
        free(p);
        return;
    }
    p->age = age;
    p->train_no = no;
    p->lower_berth = (age >= 60 && t->lower > 0);
    if (p->lower_berth)
        t->lower--;
    t->seats--;
    p->pnr = next_pnr++;
    p->next = head;
    head = p;

    printf("Ticket booked! PNR = %d, Fare = %.0f\n", p->pnr, t->fare);
    if (p->lower_berth)
        printf("Lower berth confirmed.\n");
    else if (age >= 60) {
        enqueueSenior(p->pnr, no, age);
        printf("No lower berth free. Added to the lower berth queue.\n");
    }
}

void checkPnr() {
    int pnr, pos = 1;
    struct passenger *p;
    struct waiting *t = start;

    if (!readInt("Enter PNR = ", &pnr))
        return;
    p = findPassenger(pnr);
    if (p == NULL) {
        printf("No booking found for PNR %d.\n", pnr);
        return;
    }
    printf("Name: %s | Age: %d | Train: %d\n",
           p->name, p->age, p->train_no);
    while (t != NULL && t->pnr != pnr) {
        pos++;
        t = t->next;
    }
    if (p->lower_berth)
        printf("Berth: LOWER (confirmed)\n");
    else if (t != NULL)
        printf("Berth: normal, position %d in lower berth queue\n", pos);
    else
        printf("Berth: normal (confirmed)\n");
}

void cancelTicket() {
    int pnr;
    struct passenger *p = head, *prev = NULL, *w;
    struct train *t;
    struct waiting *q;

    if (!readInt("Enter PNR to cancel = ", &pnr))
        return;
    while (p != NULL && p->pnr != pnr) {
        prev = p;
        p = p->next;
    }
    if (p == NULL) {
        printf("No booking found for PNR %d.\n", pnr);
        return;
    }
    t = findTrain(p->train_no);
    t->seats++;
    if (prev == NULL)
        head = p->next;
    else
        prev->next = p->next;
    removeFromQueue(pnr);

    if (p->lower_berth) {
        t->lower++;
        q = start;
        while (q != NULL && q->train_no != t->no)
            q = q->next;
        if (q != NULL) {
            w = findPassenger(q->pnr);
            w->lower_berth = 1;
            t->lower--;
            printf("PNR %d (age %d) moved from queue to the lower berth.\n",
                   w->pnr, w->age);
            removeFromQueue(w->pnr);
        }
    }
    free(p);
    printf("Ticket cancelled.\n");
}

int main() {
    int choice;

    insertStation("Mumbai");
    insertStation("Delhi");
    insertStation("Ahmedabad");
    insertStation("Kolkata");
    insertStation("Jaipur");

    do {
        choice = 0;
        printf("\n~~~RAILWAY MENU~~~\n");
        printf("1. Search Station\n2. Book Ticket\n3. Lower berth queue\n");
        printf("4. Check PNR\n5. Cancel Ticket\n6. Exit\n");
        printf("Enter your choice = ");
        if (scanf("%d", &choice) != 1) {
            if (feof(stdin))
                break;
            flushInput();
            printf("Invalid input!\n");
            continue;
        }
        switch (choice) {
            case 1: searchStation(); break;
            case 2: bookTicket();    break;
            case 3: showQueue();     break;
            case 4: checkPnr();      break;
            case 5: cancelTicket();  break;
            case 6: printf("Thank you for using this software!\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 6);
    return 0;
}
