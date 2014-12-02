#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>

#include <fstream>

#include "TextUI.h"
#include "InputParser.h"
#include "BankClient.h"
#include "BankMaintainer.h"
#include "BankManager.h"
#include "BankMemberDatabase.h"
#include "debug.h" 	//for debug stuff!

TextUI::TextUI(){
	bShutdown = false;
	bLogging = false;
	bank.setExecutionTraceStatus(false);
}

TextUI::~TextUI(){
	//Nothing to release.
}

void TextUI::readFromBase(){
	std::ifstream traceCheck;
	traceCheck.open("BankDatabase.txt");
	traceCheck >> std::boolalpha >> bLogging;
	traceCheck.close();

	bank_printf(bLogging, "%s on %s: readFromBase() entered\n", __TIME__, __DATE__);
	bank.readStateFromFile();	
	bank_printf(bLogging, "%s on %s: readFromBase() exited\n", __TIME__, __DATE__);
	return;
}

void TextUI::displayHeader(){
	bank_printf(bLogging, "%s on %s: displayHeader() entered\n", __TIME__, __DATE__);
	std::cout << "\n" << "****************************************************" << std::endl
		<< "\tBanking System v0.9" << std::endl
		<< "****************************************************" << std::endl << std::endl;
	bank_printf(bLogging, "%s on %s: displayHeader() exited\n", __TIME__, __DATE__);
	return;
}

BankMember* TextUI::setupFirstMaintainer(){
	std::string tFirstName;
	std::string tLastName;
	long tPIN;
	unsigned long tID;
	std::cout << "-------------------------------\n"
			  << "Setting up the first maintainer\n"
		      << "-------------------------------\n";
	std::cout << "Please enter the FIRST NAME of the first system administrator:\n";
	std::cin  >> tFirstName;
	std::cout << "Please enter the LAST NAME of the administator:\n";
	std::cin  >> tLastName; 
	std::cout << "Please enter the PIN for the administrator (4 digits, cannot start with 0):\n";
	while ((!InputParser::getLong(tPIN)) || (!isValidPIN(tPIN)))
	{
		std::cout << "Invalid PIN, please try again:\n";
	}
	tID = bank.generateNewBankMemberId();
	BankMaintainer* firstMaintainer = new BankMaintainer(tFirstName, tLastName, tID, tPIN);
	std::cout << "The new user's ID is " << firstMaintainer->getId() << ".\n";
	bank.addMaintainer(firstMaintainer);
	std::cout << "First maintainer setup.\n\n";
	return firstMaintainer;
}

BankMember* TextUI::setupFirstManager(){
	std::string tFirstName;
	std::string tLastName;
	long tPIN;
	unsigned long tID;
	std::cout << "----------------------------\n"
		      << "Setting up the first manager\n"
			  << "----------------------------\n";
	std::cout << "Please enter the FIRST NAME of the first manager:\n";
	std::cin  >> tFirstName;
	std::cout << "Please enter the LAST NAME of the manager:\n";
	std::cin  >> tLastName;
	std::cout << "Please enter the PIN for the manager (4 digits, cannot start with 0):\n";
	while ((!InputParser::getLong(tPIN)) || (!isValidPIN(tPIN)))
	{
		std::cout << "Invalid PIN, please try again:\n";
	}
	tID = bank.generateNewBankMemberId();
	BankManager* firstManager = new BankManager(tFirstName, tLastName, tID, tPIN);
	std::cout << "The new user's ID is " << firstManager->getId() << ".\n";
	bank.addManager(firstManager);
	std::cout << "First manager setup.\n\n";
	return firstManager;
}

bool TextUI::processTransaction(){
	bank_printf(bLogging, "%s on %s: processTransaction() entered\n", __TIME__, __DATE__);

	bool successful = true;
	//Get user ID
	BankMember* user = promptForId();
	//Get user to input their PIN, and confirm that they're ok
	if (!promptForPin(user)){
		bank_printf(bLogging, "%s on %s: processTransaction() exited\n", __TIME__, __DATE__);
		return true;  //Exit, but don't shutdown
	}
	bool cont = true;

	while (cont){
		//User has entered correct PIN, check user type and continue:
		if (BankManager * bm = dynamic_cast<BankManager*>(user)){
			cont = processManagerTransaction(bm);
		}
		else if (BankMaintainer * bt = dynamic_cast<BankMaintainer*>(user)){
			cont = processMaintainerTransaction(bt);
		}
		else if (BankClient * bc = dynamic_cast<BankClient*>(user)){
			cont = processClientTransaction(bc);
		}
		else{
			//Should be unreachable, will just end if user does not have a type
			bank_printf(bLogging, "%s on %s: Reached unreachable case. Invalid type.\n", __TIME__, __DATE__);
			break;
			cont = false;
		}

		//If user cancels before selecting a transation, it returns false and quits immediately,
		if (!cont){
			break;
		}

		//Otherwise they are prompted below
		char choice;
		std::cout << "\nWould you like to make another transaction?  Yes=y/Y, No=Any\n";  //Space out with extra newline
		while (cont){
			if (!InputParser::getChar(choice)){
				std::cout << "Invalid choice, please try again:\n";
				continue;
			}
			if ((choice == 'y') || (choice == 'Y')){
				cont = true;
				break;
			}
			else {
				cont = false;
				break;
			}
		}
	}

	std::cout << "Goodbye " << user->getFirstName() << "!\n";

	//Check if it is time to shut down
	if (bShutdown == true){
		successful = false;
	}

	bank_printf(bLogging, "%s on %s: processTransaction() exited\n", __TIME__, __DATE__);
	return successful;
}

bool TextUI::getShutdownStatus(){
	return bShutdown;
}

BankMember* TextUI::promptForId(){
	bank_printf(bLogging, "%s on %s: promptForID() entered\n", __TIME__, __DATE__);

	//Promt to log in.
	long lngID;
	int intIdx;
	BankMember* member = NULL;
	bool bFound = false;
	while (!bFound){
		displayHeader();
		std::cout << "Enter your ID to begin.\n";
		if (!InputParser::getLong(lngID)){
			std::cout << "Invalid ID, please try again.\n";
			continue;
		}
		if ((member = bank.getBankMember(lngID)) == NULL){
			std::cout << "\tUser with ID " << lngID << " not found.\n";
		}
		else{
			bFound = true;
		}
	}
	bank_printf(bLogging, "%s on %s: promptForID() exited\n", __TIME__, __DATE__);
	return member;
}

bool TextUI::promptForPin(BankMember* user){
	bank_printf(bLogging, "%s on %s: promptForPin(BankMember)entered\n", __TIME__, __DATE__);

	bool successful = true;
	char attempts = 0;
	long pin;
	std::cout << "Hello " << user->getFirstName() << ", please enter your PIN to login.\n";
	while ((!InputParser::getLong(pin)) || (pin != user->getPin())){
		attempts++;
		if (attempts >= 3){
			std::cout << "Maximum login attempt count exceeded.\n\n";
			bank_printf(bLogging, "%s on %s: promptForPin(BankMember)exited\n", __TIME__, __DATE__);
			return false;
		}
		std::cout << "You have entered the incorrect PIN, please try again ("
			<< attempts + 1 << "/3):\n";
	}
	bank_printf(bLogging, "%s on %s: promptForPin(BankMember)exited\n", __TIME__, __DATE__);
	return successful;
}

bool TextUI::changePIN(BankMember* user){
	bank_printf(bLogging, "%s on %s: changePin(BankMember) entered\n", __TIME__, __DATE__);

	bool successful = true, authenticated = false;
	int pin, tries, pin2;
	std::cout << "Please enter your current PIN:\n";
	tries = 0;
	do{
		while (!InputParser::getInt(pin)){
			std::cout << "Invalid entry, please try again:\n";
		}
		tries++; 
		if (pin == user->getPin()){
			authenticated = true;
			break;
		}
		else{
			std::cout << "Incorrect PIN, please try again (" << tries << "/3):\n";
			authenticated = false;
		}
	} while (tries < 3);

	if (!authenticated){
		std::cout << "Sorry, maximum number of tries met.\nPlease try again later.\n";
		bank_printf(bLogging, "%s on %s: changePin(BankMember) exited\n", __TIME__, __DATE__);
		return successful = false;
	}

	std::cout << "Please enter your new PIN (must be 4 digits):\n";
	tries = 0;
	while (!InputParser::getInt(pin)||!isValidPIN(pin)){
		tries++;
		if (tries >= 3){
			std::cout << "Sorry, maximum number of tries met.\nPlease try again later.\n";
			bank_printf(bLogging, "%s on %s: changePin(BankMember) exited\n", __TIME__, __DATE__);
			return successful = false;
		}
		else{
			std::cout << "Invalid entry, please try again:\n";
		}
	}

	std::cout << "Please enter your new PIN again to confirm:\n";
	tries = 0;
	do{
		while (!InputParser::getInt(pin2) || !isValidPIN(pin)){
			tries++;
			if (tries >= 3){
				std::cout << "Sorry, maximum number of tries met.\nPlease try again later.\n";
				bank_printf(bLogging, "%s on %s: changePin(BankMember) exited\n", __TIME__, __DATE__);
				return successful = false;
			}
			else{
				std::cout << "Invalid entry, please try again:\n";
			}
		}
		if (pin != pin2){
			tries++;
			if (tries >= 3){
				std::cout << "Sorry, maximum number of tries met.\nPlease try again later.\n";
				bank_printf(bLogging, "%s on %s: changePin(BankMember) exited\n", __TIME__, __DATE__);
				return successful = false;
			}
			std::cout << "Sorry, PINs do not match.\nPlease try again:\n";
		}
		else{
			authenticated = true;
		}
	} while (tries < 3);

	if (authenticated){
		user->setPin(pin);
		std::cout << "Pin changed.\n";
	}

	bank_printf(bLogging, "%s on %s: changePin(BankMember) exited\n", __TIME__, __DATE__);
	return successful;
}

bool TextUI::isValidPIN(int pin){
	bank_printf(bLogging, "%s on %s: isValidPin(int) entered\n", __TIME__, __DATE__);

	// Pin must be 4 digits and positive
	bool isValid = (pin < 10000) && (pin>999);
	bank_printf(bLogging, "%s on %s: isValidPin(int) exited\n", __TIME__, __DATE__);
	return isValid;
}

bool TextUI::processManagerTransaction(BankManager* user){
	bank_printf(bLogging, "%s on %s: processManagerTransaction(BankMember) entered\n", __TIME__, __DATE__);

	bool successful = true;
	std::cout 
		<< "============================\n"
		<< "  Welcome manager " << user->getFirstName() << "!\n"
		<< "============================\n"
		<< "Please select a transaction:\n"
		<< "  (1) -- Open Account\n"
		<< "  (2) -- Close Account\n"
		<< "  (3) -- Query Account\n"
		<< "  (4) -- Query Accounts\n"
		<< "  (5) -- View Bank Totals\n"
		<< "  (6) -- Cancel Transaction\n"
		<< "----------------------------\n   ";
	char choice;
	while (!InputParser::getChar(choice)||!((choice>='1')&&(choice<='5'))){
		std::cout << "Invalid choice, please try again:\n";
	}
	switch (choice){
	case '1':
		openAccount();
		break;
	case '2':
		closeAccount(user);
		break;
	case '3':
		queryAccount();
		break;
	case '4':
		queryAccounts();
		break;
	case '5':
		viewBankTotals();
		break;
	case '6':
		successful = false;
		break;
	default:
		//Should be unreachable
		bank_printf(bLogging, "%s on %s: Reached unreachable case. Invalid type.\n", __TIME__, __DATE__);
		break;
	}

	bank_printf(bLogging, "%s on %s: processManagerTransaction(BankMember) exited\n", __TIME__, __DATE__);
	return successful;
}

bool TextUI::openAccount(){
	bank_printf(bLogging, "%s on %s: openAccount() entered\n", __TIME__, __DATE__);
	bool successful = true;
	std::string firstName;
	std::string lastName;
	int PIN;
	unsigned long ID = bank.generateNewBankMemberId();
	char userType;

	std::cout << "Please enter the new user's first name:\n";
	if (!InputParser::getString(firstName)){
		std::cout << "Invalid input.  Cancelling.\n";
		bank_printf(bLogging, "%s on %s: openAccount() exited\n", __TIME__, __DATE__);
		return false;
	}

	std::cout << "Please enter the new user's last name:\n";
	if (!InputParser::getString(lastName)){
		std::cout << "Invalid input.  Cancelling.\n";
		bank_printf(bLogging, "%s on %s: openAccount() exited\n", __TIME__, __DATE__);
		return false;
	}

	std::cout << "Please enter the new user's 4-digit PIN (cannot start with '0'):\n";
	while (!InputParser::getInt(PIN) || !isValidPIN(PIN)){
		std::cout << "Invalid input.  Please try again.\n";
	}

	std::cout << "Please enter the new user's type.\n"
		<< "Client = C/c, Manager = M/m, Maintainer = T/t\n";
	while ((!InputParser::getChar(userType)) || (
		(userType != 'C') && (userType != 'c') &&
		(userType != 'M') && (userType != 'm') &&
		(userType != 'T') && (userType != 't'))){
		std::cout << "Invalid input.  Please try again:\n";
	}

	if ((userType == 'c') || (userType == 'C')){
		//Check if the user wants to open a chequing or savings accout first.
		BankClient* tempClient = new BankClient(firstName, lastName, ID, PIN);
		std::cout << "Would you like to open a chequing or savings account, or both?\n"
			<< "Chequing = C/c, Savings = S/s, Both = B/b\n";
		char choice;
		while ((!InputParser::getChar(choice)) || (
			(choice != 'C') && (choice != 'c') &&
			(choice != 'S') && (choice != 's') &&
			(choice != 'B') && (choice != 'b'))){
			std::cout << "Invalid input, please try again.\n";
		}

		bank.addClient(tempClient);

		switch (choice)
		{
		case 'C':
		case 'c':
			bank.addBankAccount(tempClient->getId(), BankAccount::CHECKING);
			break;
		case 'S':
		case 's':
			bank.addBankAccount(tempClient->getId(), BankAccount::SAVING);
			break;
		case 'B':
		case 'b':
			bank.addBankAccount(tempClient->getId(), BankAccount::CHECKING);
			bank.addBankAccount(tempClient->getId(), BankAccount::SAVING);
			break;
		default:
			bank_printf(bLogging, "%s on %s: Reached unreachable case. Invalid type.\n", __TIME__, __DATE__);
			break;  //Not reachable
		}

		std::cout << "The new user's ID is " << tempClient->getId() << ".\n";
	}
	if ((userType == 'm') || (userType == 'M')){
		BankMaintainer* tempMaintainer = new BankMaintainer(firstName, lastName, ID, PIN);
		bank.addMaintainer(tempMaintainer);
		std::cout << "The new user's ID is " << tempMaintainer->getId() << ".\n";
	}
	if ((userType == 't')||(userType == 'T')){
		BankManager* tempManager = new BankManager(firstName, lastName, ID, PIN);
		bank.addManager(tempManager);
		std::cout << "The new user's ID is " << tempManager->getId() << ".\n";
	}

	std::cout << "Account created.\n";

	bank_printf(bLogging, "%s on %s: openAccount() exited\n", __TIME__, __DATE__);
	return successful;
}

bool TextUI::closeAccount(BankManager* manager){
	bank_printf(bLogging, "%s on %s: closeAccount(BankMember) entered\n", __TIME__, __DATE__);
	bool successful = true;
	long ID;
	
	std::cout << "Which accout would you like to close?\n"
		<< "Enter their user ID.\n";
	while (!InputParser::getLong(ID)){
		std::cout << "Invalid input, please try again.\n";
	}

	//Check if the account has money in it.
	BankMember* memberToDelete = bank.database.getBankMember(ID);

	if (BankClient * clientToDelete = dynamic_cast<BankClient*>(memberToDelete)){
		if (((clientToDelete->hasChequing())&&(clientToDelete->checkChequingBalance() != 0)) 
			|| ((clientToDelete->hasSavings())&&(clientToDelete->checkSavingsBalance() != 0))){
			std::cout << "Unable to delete client - balances are not zero.\n";
			successful = false;
		}
	}
	else if (memberToDelete->getId()==manager->getId()){
		//Make sure that manager cannot close self!
		std::cout << "You cannot delete yourself!\n";
		successful = false;
	}
	else if (BankMaintainer * maintainerToDelete = dynamic_cast<BankMaintainer*>(memberToDelete)){
		//Manager cannot delete maintainers for now
		std::cout << "You cannot delete the maintainer!\n";
		successful = false;
	}
	else{
		//Unassigned, just delete
	}

	if (successful){
		successful = bank.deleteMember(ID);
	}

	if (successful){
		std::cout << "Account deleted.\n";
	}
	else{
		std::cout << "Account could NOT be deleted.\n";
		//TODO: Show the reason why not.
	}

	bank_printf(bLogging, "%s on %s: closeAccount(BankMember) exited\n", __TIME__, __DATE__);
	return successful;
}

void TextUI::queryAccount(){
	bank_printf(bLogging, "%s on %s: queryAccount() entered\n", __TIME__, __DATE__);
	//Get user
	long ID;
	std::cout << "Which accout would you like to view?\n"
		<< "Enter their user ID.\n";
	while (!InputParser::getLong(ID)){
		std::cout << "Invalid input, please try again.\n";
	}

	//Get account account has money in it.
	BankMember* memberToView = bank.getBankMember(ID);

	showAccount(memberToView);

	////Give the user some time to see the results before moving on.
	//std::cout << "Press any key then enter to contiune...";
	//std::getchar();

	//Give option to print to file?

	bank_printf(bLogging, "%s on %s: queryAccount() exited\n", __TIME__, __DATE__);
	return;
}

void TextUI::queryAccounts(){
	bank_printf(bLogging, "%s on %s: queryAccounts() entered\n", __TIME__, __DATE__);

	//Get list of accounts from bank, spam out.
	//Just list name, ID
	//iterate through all accounts and list their properties.
	//TO DO

	bank_printf(bLogging, "%s on %s: queryAccounts() exited\n", __TIME__, __DATE__);
	return;
}

void TextUI::showAccount(BankMember* memberToView){
	std::cout << "-------------------------------\n";
	if (BankClient * clientToView = dynamic_cast<BankClient*>(memberToView)){
		std::cout << "ID:                   " << clientToView->getId() << "\n";
		std::cout << "Name:                 " << clientToView->getFullName() << "\n";
		std::cout << "Type:                 " << "Client\n";
		if (clientToView->hasChequing()){
			std::cout << "Chequing account ID:  ";
			std::cout << clientToView->getAccount(BankAccount::CHECKING)->getAccountId() << "\n";
			std::cout << "Chequing Balance:     ";
			std::cout << InputParser::moneyToStr(clientToView->checkChequingBalance()) << "\n";
		}
		else{
			std::cout << "Chequing account ID:  N/A\n";
			std::cout << "Chequing Balance:     N/A\n";
		}
		if (clientToView->hasSavings()){
			std::cout << "Savings account ID:   ";
			std::cout << clientToView->getAccount(BankAccount::SAVING)->getAccountId() << "\n";
			std::cout << "Savings Balance:      ";
			std::cout << InputParser::moneyToStr(clientToView->checkSavingsBalance()) << "\n";
		}
		else{
			std::cout << "Savings account ID:   N/A\n";
			std::cout << "Savings Balance:      N/A\n";
		}
	}
	else if (BankManager * managerToView = dynamic_cast<BankManager*>(memberToView)){
		std::cout << "ID:   " << managerToView->getId() << "\n";
		std::cout << "Name: " << managerToView->getFullName() << "\n";
		std::cout << "Type: " << "Manager\n";
	}
	else if (BankMaintainer * maintainerToView = dynamic_cast<BankMaintainer*>(memberToView)){
		std::cout << "ID:   " << maintainerToView->getId() << "\n";
		std::cout << "Name: " << maintainerToView->getFullName() << "\n";
		std::cout << "Type: " << "Maintainer\n";
	}
	else{
		//Unassigned, just show name/ID
		std::cout << "ID:   " << memberToView->getId() << "\n";
		std::cout << "Name: " << memberToView->getFullName() << "\n";
	}
}

//TODO
void TextUI::viewBankTotals(){
	bank_printf(bLogging, "%s on %s: viewBankTotals() entered\n", __TIME__, __DATE__);
	std::cout << "Sorry, this feature is currently unimplemented.\n";
	bank_printf(bLogging, "%s on %s: viewBankTotals() exited\n", __TIME__, __DATE__);
	return;
}

bool TextUI::processMaintainerTransaction(BankMaintainer* user){
	bank_printf(bLogging, "%s on %s: processMaintainerTransaction(BankMember) entered\n", __TIME__, __DATE__);

	bool cont = true;
	std::cout 
		<< "============================\n"
		<< "  Welcome maintainer " << user->getFirstName() << "!\n"
		<< "============================\n"
		<< "Please select a transaction:\n"
		<< "  (1) -- Enable/Disable Logging\n"
		<< "  (2) -- Clear Logs\n"
		<< "  (3) -- Print Logs\n"
		<< "  (4) -- Shutdown\n"
		<< "  (5) -- Cancel Transaction\n"
		<< "----------------------------\n   ";
	char choice;
	while (!InputParser::getChar(choice) || !((choice >= '1')&&(choice <= '4'))){
		std::cout << "Invalid choice, please try again.\n\n";
	}
	switch (choice){
	case '1':
		enableLogging();
		break;
	case '2':
		removeLogs();
		break;
	case '3':
		printLogs();
		break;
	case '4':
		cont = shutdown();
		break;
	case '5':
		cont = false;
		break;
	default:
		//Should be unreachable
		bank_printf(bLogging, "%s on %s: Reached unreachable case. Invalid type.\n", __TIME__, __DATE__);
		break;
	}
	bank_printf(bLogging, "%s on %s: processMaintainerTransaction(BankMember) exited\n", __TIME__, __DATE__);
	return cont;
}

bool TextUI::enableLogging(){
	bank_printf(bLogging, "%s on %s: enableLogging() entered\n", __TIME__, __DATE__);

	bool successful = true;
	char chrEn;

	// Check if the logging level is on
	std::cout << "Logging is currently " << (bLogging ? "enabled" : "disabled") << ".\n";
	std::cout << "Would you like to enable or disable logging?\n";
	std::cout << "E/e to enable, D/d to disable:\n";

	while ((!InputParser::getChar(chrEn)) || !((chrEn=='E')||(chrEn=='e')||(chrEn=='D')||(chrEn=='d'))){
		std::cout << "Invalid input, please try again:\n";
	}

	if ((chrEn == 'E') || (chrEn == 'e')){
		bank.setExecutionTraceStatus(true);
		bLogging = true;
		std::cout << "Logging enabled.\n";
	}
	else{
		bank.setExecutionTraceStatus(false);
		bLogging = false;
		std::cout << "Logging disabled.\n";
	}
	bank_printf(bLogging, "%s on %s: enableLogging() exited\n", __TIME__, __DATE__);
	return successful;
}

bool TextUI::removeLogs(){
	bank_printf(bLogging, "%s on %s: removeLogs() entered\n", __TIME__, __DATE__);
	bool successful;

   	std::ifstream inputFile;
	inputFile.open("trace.txt");
	if (inputFile.is_open()){
		successful = true;
		remove("trace.txt");	
		std::cout << "Execution trace cleared.\n";
	}
	else {
		successful = false;
		std::cout << "No execution trace currently exists.\n";
	}

	inputFile.close();
	bank_printf(bLogging, "%s on %s: removeLogs() exited\n", __TIME__, __DATE__);
	return successful;
}

bool TextUI::printLogs(){
	bank_printf(bLogging, "%s on %s: printLogs() entered\n", __TIME__, __DATE__);
	bool successful;

	// Open the file
    	std::ifstream inputFile;
    	inputFile.open("trace.txt");
    	if (inputFile.is_open()){
		successful = true;
		std::string line;
		size_t pos;
		while (std::getline(inputFile, line)){
			std::cout << line << std::endl;
		}
	}
	else {
		successful = false;
		std::cout << "No execution trace currently exists.\n";
	}

	inputFile.close();
	bank_printf(bLogging, "%s on %s: printLogs() exited\n", __TIME__, __DATE__);
	return successful;
}

bool TextUI::shutdown(){
	bank_printf(bLogging, "%s on %s: shutdown() entered\n", __TIME__, __DATE__);
	bool cont = true;
	char choice;
	std::cout << "Are you sure that you want to shutdown?\n"
		<< "Yes = Y/y, No = Any other\n";
	while (!InputParser::getChar(choice)){
		std::cout << "Invalid input, please try again:\n";
	}
	if ((choice == 'Y') || (choice == 'y')){
		std::cout << "Shutting down.\n";
		bank.writeStateToFile(); //Save bank data
		bShutdown = true;       //Raise shutdown flag
		cont = false;          //Don't perform another transaction, just shut down
	}
	else{
		std::cout << "Cancelling shutdown request.\n";
		bShutdown = false;
	}

	bank_printf(bLogging, "%s on %s: shutdown() exited\n", __TIME__, __DATE__);
	return cont;
}

bool TextUI::processClientTransaction(BankClient* user){
	bank_printf(bLogging, "%s on %s: processClientTransaction(BankMember) entered\n", __TIME__, __DATE__);

	bool cont = true;
	//Check if there is a complimentary account
	bool bMissingComplimentary = !((user->hasChequing())&&(user->hasSavings()));
	int i = 5;
	std::cout 
		<< "============================\n"
		<< "  Welcome user " << user->getFirstName() << "!\n"
		<< "============================\n"
		<< "Please select a transaction:\n"
		<< "  (1) -- Withdrawal\n"
		<< "  (2) -- Deposit\n"
		<< "  (3) -- Transfer\n"
		<< "  (4) -- View Balances\n";
	//Check if the user is missing a chequing or savings account and give them an option to open one.
	if (bMissingComplimentary){
		if (!user->hasChequing()){
			std::cout << "  (5) -- Open Chequing Account\n";
		}
		else{
			std::cout << "  (5) -- Open Savings Account\n";
		}
		std::cout << "  (6) -- Cancel Transaction\n";
	}
	else{
		std::cout << "  (5) -- Cancel Transaction\n";
	}
	std::cout << "----------------------------\n   ";
	char choice;
	bool bBreak;
	while (!InputParser::getChar(choice) || !((choice >= '1') && (choice <= '5'))){
		if ((choice==6)&&(!bMissingComplimentary)){ //Add complimentary not valid if user has both already
			std::cout << "Invalid choice, please try again.\n\n";
			continue;
		}
		else if (choice == '6'){
			//If user presses 6 and does not have missing accounts, they have cancelled
			break;
		}
	}
	switch (choice){
	case '1':
		withdrawal(user);
		break;
	case '2':
		deposit(user);
		break;
	case '3':
		transfer(user);
		break;
	case '4':
		balances(user);
		break;
	case '5':
		if (bMissingComplimentary){
			openComplimentary(user);
		}
		else{
			cont = false;
		}
		break;
	case '6':
		cont = false;
		break;
	default:
		//Should be unreachable
		bank_printf(bLogging, "%s on %s: Reached unreachable case. Invalid type.\n", __TIME__, __DATE__);
		break;
	}
	
	bank_printf(bLogging, "%s on %s: processClientTransaction(BankMember) exited\n", __TIME__, __DATE__);
	return cont;
}

long TextUI::withdrawal(BankClient* client){
	bank_printf(bLogging, "%s on %s: withdrawal(BankClient*) entered\n", __TIME__, __DATE__);

	long amount = 0;
	//Print out balances
	balances(client);
	//Ask which account to withdraw from
	bool hasChequing = client->hasChequing();
	bool hasSavings  = client->hasSavings();
	bool fromChequing;
	char choice;

	std::cout << "Withdraw from:\n";
	if ((hasChequing) && (hasSavings)){
		std::cout << "  (1) - Chequing\n"
			<< "  (2) - Savings\n";
		while (!InputParser::getChar(choice) || !((choice == '1') || (choice == '2'))){
			std::cout << "Invalid choice. Please try again:\n";
		}
		if (choice == '1'){
			fromChequing = true;
		}
		else{
			fromChequing = false;
		}
	}
	else if (hasChequing){
		std::cout << "  (1) - Chequing\n";
		while (!InputParser::getChar(choice) || choice != '1'){
			std::cout << "Invalid choice. Please try again:\n";
		}
		fromChequing = true;
	}
	else{ // hasSavings must be true
		std::cout << "  (1) - Savings\n";
		while (!InputParser::getChar(choice) || choice != '1'){
			std::cout << "Invalid choice. Please try again:\n";
		}
		fromChequing = false;
	}

	//Get Amount
	std::cout << "How much would you like to withdraw?\n";
	while (!InputParser::getMoney(amount)){
		std::cout << "Invalid entry, please try again:\n";
	}

	//Actually do withdrawal
	if (fromChequing){
		amount = removeMoneyFromChequing(client, amount);
	}
	else{
		amount = removeMoneyFromSavings(client, amount);
	}

	if (fromChequing){
		std::cout << InputParser::moneyToStr(amount) << " withdrawn from chequing account.\n";
		std::cout << "Your new chequing balance is " << InputParser::moneyToStr(client->checkChequingBalance()) << ".\n";
	}
	else{
		std::cout << InputParser::moneyToStr(amount) << " withdrawn from savings account.\n";
		std::cout << "Your new savings balance is " << InputParser::moneyToStr(client->checkSavingsBalance()) << ".\n";
	}

	bank_printf(bLogging, "%s on %s: withdrawal(BankClient*) exited\n", __TIME__, __DATE__);
	return amount;
}

long TextUI::deposit(BankClient* client){
	bank_printf(bLogging, "%s on %s: deposit(BankClient*) entered\n", __TIME__, __DATE__);
	long amount = 0;
	//Print out balances
	balances(client);
	//Ask which account to withdraw from
	bool hasChequing = client->hasChequing();
	bool hasSavings = client->hasSavings();
	bool toChequing;
	char choice;

	std::cout << "Deposit to:\n";
	if ((hasChequing) && (hasSavings)){
		std::cout << "  (1) - Chequing\n"
			<< "  (2) - Savings\n";
		while (!InputParser::getChar(choice) || !((choice == '1') || (choice == '2'))){
			std::cout << "Invalid choice. Please try again:\n";
		}
		if (choice == '1'){
			toChequing = true;
		}
		else{
			toChequing = false;
		}
	}
	else if (hasChequing){
		std::cout << "  (1) - Chequing\n";
		while (!InputParser::getChar(choice) || !(choice == '1')){
			std::cout << "Invalid choice. Please try again:\n";
		}
		toChequing = true;
	}
	else{ // hasSavings must be true
		std::cout << "  (1) - Savings\n";
		while (!InputParser::getChar(choice) || !(choice == '1')){
			std::cout << "Invalid choice. Please try again:\n";
		}
		toChequing = false;
	}

	//Actually do deposit
	std::cout << "How much would you like to deposit?\n";
	while (!InputParser::getMoney(amount)){
		std::cout << "Invalid input, please try again.";
	}

	if (toChequing){
		client->getAccount(BankAccount::CHECKING)->deposit(amount);
	}
	else{
		client->getAccount(BankAccount::SAVING)->deposit(amount);
	}

	bank_printf(bLogging, "%s on %s: deposit(BankClient*) exited\n", __TIME__, __DATE__);
	return amount;
}

long TextUI::transfer(BankClient* client){
	bank_printf(bLogging, "%s on %s: transfer(BankClient*) entered\n", __TIME__, __DATE__);
	long amount = 0;
	long amountTransfered = 0;
	//Print out balances
	balances(client);
	//Ask which account to withdraw from
	bool hasChequing = client->hasChequing();
	bool hasSavings = client->hasSavings();
	bool fromChequing;
	enum target_t { CHEQUING, SAVINGS, OTHER };
	target_t trTo;
	long otherID;
	BankMember* otherMember;
	BankClient* otherClient = NULL;

	char choice;

	std::cout << "Transfer from:\n";
	if ((hasChequing) && (hasSavings)){
		std::cout << "  (1) - Chequing\n"
			<< "  (2) - Savings\n   ";
		while (!InputParser::getChar(choice) || !((choice == '1') || (choice == '2'))){
			std::cout << "Invalid choice. Please try again:\n";
		}
		if (choice == '1'){
			fromChequing = true;
		}
		else{
			fromChequing = false;
		}
	}
	else if (hasChequing){
		std::cout << "  (1) - Chequing\n   ";
		while (!InputParser::getChar(choice) || !(choice == '1')){
			std::cout << "Invalid choice. Please try again:\n";
		}
		fromChequing = true;
	}
	else{ // hasSavings must be true
		std::cout << "  (1) - Savings\n   ";
		while (!InputParser::getChar(choice) || !(choice == '1')){
			std::cout << "Invalid choice. Please try again:\n";
		}
		fromChequing = false;
	}

	//Get what to transfer to
	std::cout << "Transfer to:\n";
	if ((hasChequing) && (hasSavings)){
		if (fromChequing){
			std::cout << "  (1) - Savings\n";
		}
		else{
			std::cout << "  (1) - Chequing\n";
		}
		std::cout << "  (2) - To Other's Chequing Account\n   ";
		while (!InputParser::getChar(choice) || !((choice == '1') || (choice == '2'))){
			std::cout << "Invalid choice. Please try again:\n";
		}
		if (choice == '1'){
			if (fromChequing){
				trTo = SAVINGS;
			}
			else{
				trTo = CHEQUING;
			}
		}
		else{
			trTo = OTHER;
		}
	}
	else{
		std::cout << "  (1) - To Other's Chequing Account\n   ";
		while (!InputParser::getChar(choice) || !((choice == '1'))){
			std::cout << "Invalid choice. Please try again:\n";
		}
		//Only one choice...
		trTo = OTHER;
	}
	if (trTo == OTHER){
		//Get other user by ID.
		std::cout << "Which account ID would you like to transfer to?\n";
		char tries = 0;
		while (!InputParser::getLong(otherID)){
			tries++;
			if (tries < 2){
				std::cout << "Invalid entry, please try again (" << tries << "/3):\n";
			}
			else{
				std::cout << "Maximum number of attempts made.\nPlease try again later.\n";
				bank_printf(bLogging, "%s on %s: transfer(BankClient*) exited\n", __TIME__, __DATE__);
				return amount = 0;
			}
		}
		if (((otherMember = bank.getBankMember(otherID)) == NULL) || ((otherClient = dynamic_cast<BankClient*>(otherMember)) == NULL)){
			std::cout << "Client " << otherID << " does not exist.\nPlease try again later.\n";
			bank_printf(bLogging, "%s on %s: transfer(BankClient*) exited\n", __TIME__, __DATE__);
			return amount = 0;
		}
		if (!otherClient->hasChequing()){
			std::cout << "Sorry, client " << otherID << " does not have a chequing account to recieve your funds.\n"
				<< "Your transaction cannot be completed at this time.\n";
			bank_printf(bLogging, "%s on %s: transfer(BankClient*) exited\n", __TIME__, __DATE__);
			return 0;
		}
	}

	//Perform transfer
	std::cout << "How much would you like to transfer from ";
	if (fromChequing){
		std::cout << "chequing";
	}
	else{
		std::cout << "savings";
	}
	std::cout << " to ";
	switch (trTo){
	case CHEQUING:
		std::cout << "chequing";
		break;
	case SAVINGS:
		std::cout << "savings";
		break;
	case OTHER:
		std::cout << "account " << otherID;
		break;
	default: 
		//Unreachable
		bank_printf(bLogging, "%s on %s: Reached unreachable case. Invalid type.\n", __TIME__, __DATE__);
		break;
	}
	std::cout << "?\n";

	while (!InputParser::getMoney(amount)){
		std::cout << "Invalid input, please try again:\n";
	}

	if (amount > 0){
		if (fromChequing){
			amount = removeMoneyFromChequing(client, amount);
		}
		else{
			amount = removeMoneyFromSavings(client, amount);
		}

		if (amount > 0){
			//Add money to other account
			switch (trTo)
			{
			case CHEQUING:
				amountTransfered = client->getAccount(BankAccount::CHECKING)->deposit(amount);
				break;
			case SAVINGS:
				amountTransfered = client->getAccount(BankAccount::SAVING)->deposit(amount);
				break;
			case OTHER:
				amountTransfered = otherClient->getAccount(BankAccount::CHECKING)->deposit(amount);
				break;
			default:
				break;
			}
			if (amount != amountTransfered){
				//Do something, this is bad
				std::cout << "Transfer error!";
			}
		}
	}

	std::cout << "Transfer of " << InputParser::moneyToStr(amount) << " from ";
	if (fromChequing){
		std::cout << "chequing";
	}
	else{
		std::cout << "savings";
	}
	std::cout << " to ";
	switch (trTo){
	case CHEQUING:
		std::cout << "chequing";
		break;
	case SAVINGS:
		std::cout << "savings";
		break;
	case OTHER:
		std::cout << "account " << otherID;
		break;
	default: 
		//Unreachable
		bank_printf(bLogging, "%s on %s: Reached unreachable case. Invalid type.\n", __TIME__, __DATE__);
		break;
	}
	std::cout << " completed.\n";

	bank_printf(bLogging, "%s on %s: transfer(BankClient*) exited\n", __TIME__, __DATE__);
	return amount;
}

void TextUI::balances(BankClient* client){
	bank_printf(bLogging, "%s on %s: balances(BankClient*) entered\n", __TIME__, __DATE__);

	bool hasChequing = client->hasChequing();
	bool hasSavings = client->hasSavings();

	std::cout << "Balance(s):\n";
	if (hasChequing){
		std::cout << "  Chequing: " << InputParser::moneyToStr(client->checkChequingBalance()) << "\n";
	}
	if (hasSavings){
		std::cout << "  Savings:  " << InputParser::moneyToStr(client->checkSavingsBalance()) << "\n";
	}
	bank_printf(bLogging, "%s on %s: balances(BankClient*) exited\n", __TIME__, __DATE__);
	return;
}

bool TextUI::openComplimentary(BankClient* client){
	bank_printf(bLogging, "%s on %s: openComplimentary(BankClient*) entered\n", __TIME__, __DATE__);

	bool hasChequing = client->hasChequing();
	bool hasSavings  = client->hasSavings();
	bool successful = true;
	if (hasChequing && hasSavings){ // Can't open complimentary already has both, should be unreachable
		bank_printf(bLogging, "%s on %s: openComplimentary(BankClient*) exited\n", __TIME__, __DATE__);
		return false;
	}
	if (hasChequing){
		//Open savings account
		bank.addBankAccount(client->getId(), BankAccount::SAVING);
		//client->openSavings(bank.generateNewBankAccountId());  //old way
		std::cout << "Savings account opened.\n";
	}
	else{
		//Open chequing account
		bank.addBankAccount(client->getId(), BankAccount::CHECKING);
		//client->openChequing(bank.generateNewBankAccountId());  //old way
		std::cout << "Chequing account opened.\n";
	}

	bank_printf(bLogging, "%s on %s: openComplimentary(BankClient*) exited\n", __TIME__, __DATE__);
	return successful;
}

long TextUI::removeMoneyFromChequing(BankClient* client, long amount){
	bank_printf(bLogging, "%s on %s: removeMoneyFromChequing(BankClient*, long) entered\n", __TIME__, __DATE__);

	if (amount > client->checkChequingBalance()){
		std::cout << "Error: Insufficient funds.\n";
		amount=0;
	}
	else if ((client->checkChequingBalance() > 100000) && ((client->checkChequingBalance()-amount) < 100000)){
		char choice;
		std::cout << "Warning: this transaction will cause your chequing balance to fall below $1000.00.\n"
			<< "This transaction, and all subsequent transactions removing money from your chequing account while it contains less than $1000.00 "
			<< "will cost an additonal fee of $2.00.\n"
			<< "Are you sure you want to continue?\n"
			<< "Yes = Y/y, No = Any other";
		while (!InputParser::getChar(choice)){
			std::cout << "Invalid input, please try again:\n";
		}
		if ((choice == 'Y') || (choice == 'y')){
            client->getAccount(BankAccount::CHECKING)->withdrawal(amount + 200);
		}
		else{
			amount = 0;
		}
	}
	else if (client->checkChequingBalance() < 100000){
		char choice;
		std::cout << "Warning: your chequing balance is below $1000.00.\n"
			<< "This transaction will cost an additonal fee of $2.00.\n"
			<< "Are you sure you want to continue?\n"
			<< "Yes = Y/y, No = Any other";
		while (!InputParser::getChar(choice)){
			std::cout << "Invalid input, please try again:\n";
		}
		if ((choice == 'Y') || (choice == 'y')){
            client->getAccount(BankAccount::CHECKING)->withdrawal(amount + 200);
		}
		else{
			amount = 0;
		}
	}
	else { //Has enough money, won't incur an extra fee
        client->getAccount(BankAccount::CHECKING)->withdrawal(amount);
	}

	bank_printf(bLogging, "%s on %s: removeMoneyFromChequing(BankClient*, long) exited\n", __TIME__, __DATE__);
	return amount;
}

long TextUI::removeMoneyFromSavings(BankClient* client, long amount){
	bank_printf(bLogging, "%s on %s: removeMoneyFromSavings(BankClient*, long) entered\n", __TIME__, __DATE__);

	if (amount > client->checkSavingsBalance()){
		std::cout << "Error: Insufficient funds.\n";
		amount = 0;
	}
	else { //Has enough money
        client->getAccount(BankAccount::SAVING)->withdrawal(amount);
	}
	bank_printf(bLogging, "%s on %s: removeMoneyFromSavings(BankClient*, long) exited\n", __TIME__, __DATE__);
	return amount;
}

