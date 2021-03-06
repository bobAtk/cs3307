#include "BankMemberDatabase.h"

void printAccountDetails(BankClient* bc, BankAccount::AccountType accountType);

BankMemberDatabase::BankMemberDatabase() {

}

// return enum for user type, take in id.
//BankMember::MemberType getMemberType(unsigned long id) {
//	BankMemberItr bankMemberItr = bankMembers.find(id);
//	if (bankMemberItr != bankMembers.end()) {
//		return bankMemberItr->second.getMemberType();
//	}
//	return BankMember::UNASSIGNED;
//}

BankMember* BankMemberDatabase::getBankMember(unsigned long id) {
	BankMemberItr bankMemberItr = bankMembers.find(id);
	if (bankMemberItr != bankMembers.end()) {
		return (bankMemberItr->second);
	}
	return NULL;
}

bool BankMemberDatabase::addBankMember(BankMember* bankMember) {
	bankMembers.insert(MemberPair(bankMember->getId(), bankMember));
	if (bankMember->getMemberType() == BankMember::CLIENT) {
		//if (BankClient* client = dynamic_cast<BankClient*>(bankMember)) {
		//	BankAccount* ba = NULL;
		//	if (client->hasChequing()) {
		//		ba = client->getAccount(BankAccount::CHECKING);
		//		bankAccounts.insert(AccountPair(ba->getAccountId(), ba));
		//	}
		//	if (client->hasSavings()) {
		//		ba = client->getAccount(BankAccount::SAVING);
		//		bankAccounts.insert(AccountPair(ba->getAccountId(), ba));
		//	}
		//}
	}
	return true;
}

bool BankMemberDatabase::deleteBankMember(unsigned long id) {
	BankMemberItr bankMemberItr = bankMembers.find(id);
	if (bankMemberItr != bankMembers.end()) {
		//BankMember deletedMember = BankMember(bankMemberItr->second);
		bankMembers.erase(bankMemberItr);
		return true;
	}
	return false;
}

bool BankMemberDatabase::addBankAccount(unsigned long bankMemberId, BankAccount::AccountType accountType) {
	unsigned long id = BankMemberDatabase::generateNewBankAccountId();
	if (BankClient* member = dynamic_cast<BankClient*>(getBankMember(bankMemberId))) {
		member->addAccount(accountType, id);
		bankAccounts.insert(AccountPair(id, member->getAccount(accountType)));
		return true;
	}
	return false;
}

bool BankMemberDatabase::deleteBankAccount(unsigned long id) {
	BankAccountItr bankAccountItr = bankAccounts.find(id);
	if (bankAccountItr != bankAccounts.end()) {
		//BankMember deletedMember = BankMember(bankMemberItr->second);
		bankAccounts.erase(bankAccountItr);
		return true;
	}
	return false;
}

void BankMemberDatabase::loadDatabaseFromFile(std::ifstream& is) {

	int memberType;

	while (is >> memberType) {

		switch (memberType) {
			case 1:
			{
					  BankClient* bankClient = new BankClient(is, memberType);
					  bankMembers.insert(MemberPair(bankClient->getId(), bankClient));
			}
				break;
			case 0:
			{
					  BankManager* bankManager = new BankManager(is, memberType);
					  bankMembers.insert(MemberPair(bankManager->getId(), bankManager));
			}
				break;
			case 2:
			{
					  BankMaintainer* bankMaintainer = new BankMaintainer(is, memberType);
					  bankMembers.insert(MemberPair(bankMaintainer->getId(), bankMaintainer));
			}
				break;
			default:
				break;
		}
	}

}

void BankMemberDatabase::writeDatabaseToFile(std::ofstream& os) {
	for (BankMemberItr itr = bankMembers.begin(); itr != bankMembers.end(); ++itr) {
		BankMember* a = itr->second;

		//os << a->getMemberType() << " " << a->getId() << " " << a->getPin() << " " << a->getFullName() << " ";
		//os << "T:" << a->getMemberType() << " id:" << a->getId() << " pin:" << a->getPin() << " name:" << a->getFullName() << " ";
		os << a->getMemberType() << " " << a->getId() << " " << a->getPin() << " " << a->getFullName() << " ";

		
		if (a->getMemberType() == BankMember::CLIENT) {
			if (BankClient * bc = dynamic_cast<BankClient*>(a)) {
				BankAccount* ba = NULL;
				int numAccounts = bc->hasChequing() + bc->hasSavings();
				//os << "numacc: " << numAccounts << " ";
				os << numAccounts << " ";
				if (bc->hasChequing()) {
					ba = bc->getAccount(BankAccount::CHECKING);
					//os << ba->getAccountType() << " " << ba->getAccountId() << " " << ba->getBalance() << " ";
					//os << "at:" << ba->getAccountType() << " aid:" << ba->getAccountId() << " bal:" << ba->getBalance() << " ";
					os << ba->getAccountType() << " " << ba->getAccountId() << " " << ba->getBalance() << " ";
				}
				if (bc->hasSavings()) {
					ba = bc->getAccount(BankAccount::SAVING);
					//os << ba->getAccountType() << " " << ba->getAccountId() << " " << ba->getBalance() << " ";
					//os << "at:" << ba->getAccountType() << " aid:" << ba->getAccountId() << " bal:" << ba->getBalance() << " ";
					os << ba->getAccountType() << " " << ba->getAccountId() << " " << ba->getBalance() << " ";
				}
			}
		}
		os << std::endl;
	}
}

void printAccountDetails(BankClient* bc, BankAccount::AccountType accountType, std::ofstream& os) {
	BankAccount* ba = bc->getAccount(accountType);
	//os << "at:" << ba->getAccountType() << " aid:" << ba->getAccountId() << " bal:" << ba->getBalance() << " ";
	os << " " << ba->getAccountType() << " " << ba->getAccountId() << ba->getBalance() << " ";
}

unsigned long BankMemberDatabase::generateNewBankMemberId() {
	return IdManager::generateNewUniqueId(bankMembers);
}

unsigned long BankMemberDatabase::generateNewBankAccountId() {
	return IdManager::generateNewUniqueId(bankAccounts);
}

