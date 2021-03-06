#include "catch.hpp"
#include "inventory.h"
#include "product.h"
#include "register.h"

#include <memory>

using std::make_shared;
using std::shared_ptr;

TEST_CASE("assignInventory assigns an inventory object to the register", "[register]") {
	Register testRegister;
	shared_ptr<Inventory> testInventoryPointer = make_shared<Inventory>();

	REQUIRE(!testRegister.getInventory());

	testRegister.assignInventory(testInventoryPointer);

	REQUIRE(testRegister.getInventory());
}

TEST_CASE("scanItem is passed a product name, increases quantity of product and total cost if product is in register's set inventory, and returns a bool as an indicator of success/fail", "[register]") {
	Register testRegister;

	SECTION("scanItem returns false if no inventory object is currently set", "[register]") {
		REQUIRE(testRegister.scanItem("corn") == false);
	}

	shared_ptr<Inventory> testInventoryPointer = make_shared<Inventory>();
	testInventoryPointer->insert(make_shared<Product>("beef", 799));
	testRegister.assignInventory(testInventoryPointer);

	SECTION("scanItem returns true if inventory object is set and product is found in inventory", "[register]") {
		REQUIRE(testRegister.scanItem("beef") == true);
	}
	SECTION("scanItem returns false if inventory object is set and product is not found in inventory", "[register]") {
		REQUIRE(testRegister.scanItem("chips") == false);
	}
	SECTION("scanItem increases the quantity of the product in register's quantity object if product is found in register's inventory", "[register]") {
		testRegister.scanItem("beef");
		
		REQUIRE(testRegister.getQuantity("beef") == 1);
	}
	SECTION("scanItem increases the total of the register by the price of the scanned product", "[register]") {
		int curTotal = testRegister.getTotal();
		testRegister.scanItem("beef");

		REQUIRE(testRegister.getTotal() == curTotal + 799);
	}
	SECTION("scanItem inceases the total of the register by the price of the scanned product less markdown when markdown is set") {
		shared_ptr<Product> prodPtr = testRegister.getInventory()->retrieve("beef");
		prodPtr->setMarkdown(100);
		int curTotal = testRegister.getTotal();
		testRegister.scanItem("beef");

		REQUIRE(testRegister.getTotal() == curTotal + 699);
	}
}

TEST_CASE("scanItem accepts an additional second parameter indicating the weight of the product being scanned in", "[register]") {
	shared_ptr<Inventory> testInventoryPtr = make_shared<Inventory>();
	testInventoryPtr->insert(make_shared<Product>("ham", 376, true));
	testInventoryPtr->insert(make_shared<Product>("chocolate", 199));
	Register testRegister;
	testRegister.assignInventory(testInventoryPtr);
	testRegister.scanItem("ham", 110);
	
	SECTION("scanItem increases total for weighed items based on weight of product scanned") {
		REQUIRE(testRegister.getTotal() == (int) (376 * (110.0 / 100) +.5));
	}

	SECTION("scanItem increases quantity for weighed items based on weight of product scanned") {
		REQUIRE(testRegister.getQuantity("ham") == 110);
	}

	SECTION("scanItem returns false if weighed product is entered without weight") {
		bool res = testRegister.scanItem("ham");
		
		REQUIRE(res == false);
	}

	SECTION("scanItem ignores weight parameter if scanned product is entered with weight and product's byWeight is false") {
		int quan = testRegister.getQuantity("chocolate");
		int tot = testRegister.getTotal();
		bool res = testRegister.scanItem("chocolate", 29);

		REQUIRE(testRegister.getQuantity("chocolate") == quan + 1);
		REQUIRE(testRegister.getTotal() == tot + 199);
		REQUIRE(res == true);
	}
	SECTION("scanItem increases total by the price per pound less markdown times weight scanned if markdown is set") {
		shared_ptr<Product> prodPtr = make_shared<Product>("steak", 899, true);
		prodPtr->setMarkdown(179);
		testInventoryPtr->insert(prodPtr);
		int tot = testRegister.getTotal();
		testRegister.scanItem("steak", 204);

		REQUIRE(testRegister.getTotal() == tot + (int) (720 * (204 / 100.0) + .5));
	}
}

TEST_CASE("removeItem removes a product from the register and reduces the total by the corresponding amount", "[register]") {
	shared_ptr<Inventory> testInventoryPtr = make_shared<Inventory>();
	testInventoryPtr->insert(make_shared<Product>("milk", 799));
	testInventoryPtr->insert(make_shared<Product>("watermelon", 50, true));
	Register testRegister;
	testRegister.assignInventory(testInventoryPtr);
	
	SECTION("removeItem returns false if passed product has quantity of 0") {
		bool res = testRegister.removeItem("milk");
	
		REQUIRE(res == false);
	}
	SECTION("removeItem returns true if passed product has a quantity above 0") {
		testRegister.scanItem("milk");
		bool res = testRegister.removeItem("milk");

		REQUIRE(res == true);
	}
	SECTION("removeItem reduces quantity of product by 1 if product has a quantity above 0") {
		testRegister.scanItem("milk");
		int milkQuantity = testRegister.getQuantity("milk");
		testRegister.removeItem("milk");

		REQUIRE(testRegister.getQuantity("milk") == milkQuantity - 1);
	}
	SECTION("removeItem reduces total by cost of 1 unit if product has a quantity above 0") {
		testRegister.scanItem("milk");
		int tot = testRegister.getTotal();
		testRegister.removeItem("milk");

		REQUIRE(testRegister.getTotal() == tot - 799);
	}
	SECTION("removeItem ignores passed weight if passed product is not priced by weight") {
		testRegister.scanItem("milk");
		int milkQuantity = testRegister.getQuantity("milk");
		int tot = testRegister.getTotal();
		bool res = testRegister.removeItem("milk", 65);

		REQUIRE(testRegister.getQuantity("milk") == milkQuantity - 1);
		REQUIRE(testRegister.getTotal() == tot - 799);
		REQUIRE(res == true);
	}
	SECTION("removeItem returns false if passed product is priced by weight and no weight is passed") {
		testRegister.scanItem("watermelon", 207);
		bool res = testRegister.removeItem("watermelon");

		REQUIRE(res == false);
	}
	SECTION("removeItem returns false if passed product is priced by weight and weight passed is greater than amount in productList") {
		testRegister.scanItem("watermelon", 513);
		int quant = testRegister.getQuantity("watermelon");
		bool res = testRegister.removeItem("watermelon", quant + 100);

		REQUIRE(res == false);
	}
	SECTION("removeItem decreases quantity of product by passed weight amount if product is priced by weight") {
		testRegister.scanItem("watermelon", 313);
		int quantity = testRegister.getQuantity("watermelon");
		testRegister.removeItem("watermelon", 107);

		REQUIRE(testRegister.getQuantity("watermelon") == quantity - 107);
	}
	SECTION("removeItem decreases total by adjusted price if passed product is priced by weight and a weight amount is passed") {
		testRegister.scanItem("watermelon", 509);
		int tot = testRegister.getTotal();
		testRegister.removeItem("watermelon", 216);

		REQUIRE(testRegister.getTotal() == tot - (int) (50 * (216 / 100.0) + .5));
	}
	SECTION("removeItem decreases total by price less markdown if pass product has a markdown") {
		shared_ptr<Product> prodPtr = make_shared<Product>("eggs", 299);
		prodPtr->setMarkdown(72);
		testInventoryPtr->insert(prodPtr);
		testRegister.scanItem("eggs");
		int tot = testRegister.getTotal();
		testRegister.removeItem("eggs");

		REQUIRE(testRegister.getTotal() == tot - 227);
	}
	SECTION("removeItem decreases total by price less markdown times passed weight if product has a markdown and is priced by weight") {
		shared_ptr<Product> prodPtr = make_shared<Product>("orange", 199, true);
		prodPtr->setMarkdown(39);
		testInventoryPtr->insert(prodPtr);
		testRegister.scanItem("orange", 56);
		int tot = testRegister.getTotal();
		testRegister.removeItem("orange", 21);

		REQUIRE(testRegister.getTotal() == tot - (int) (160 * (21 / 100.0) + .5));
	}
}

TEST_CASE("calcPrice calculates the price correctly when the scanned item has an associated special") {
		shared_ptr<Inventory> testInventory = make_shared<Inventory>();
		shared_ptr<Product> prodPtr = make_shared<Product>("fish", 598);
		shared_ptr<Special> specPtr = make_shared<SpecialBogo>(1, 1, 70);
		prodPtr->assignSpecial(specPtr);
		testInventory->insert(prodPtr);
		prodPtr = make_shared<Product>("cereal", 299);
		specPtr = make_shared<SpecialBogo>(3, 2, 100);
		prodPtr->assignSpecial(specPtr);
		testInventory->insert(prodPtr);
		prodPtr = make_shared<Product>("coke", 499);
		specPtr = make_shared<SpecialBulk>(4, 1200);
		prodPtr->assignSpecial(specPtr);
		testInventory->insert(prodPtr);
		prodPtr = make_shared<Product>("bread", 235);
		specPtr = make_shared<SpecialBulk>(3, 500, 3);
		prodPtr->assignSpecial(specPtr);
		testInventory->insert(prodPtr);
		prodPtr = make_shared<Product>("cheese", 199);
		specPtr = make_shared<SpecialBogo>(3, 1, 100, 4);
		prodPtr->assignSpecial(specPtr);
		testInventory->insert(prodPtr);
		prodPtr = make_shared<Product>("bacon", 700, true);
		specPtr = make_shared<SpecialBogo>(200, 100, 50);
		prodPtr->assignSpecial(specPtr);
		testInventory->insert(prodPtr);
		prodPtr = make_shared<Product>("shrimp", 500, true);
		specPtr = make_shared<SpecialBogo>(200, 200, 75, 400);
		prodPtr->assignSpecial(specPtr);
		testInventory->insert(prodPtr);
		Register testRegister;
		testRegister.assignInventory(testInventory);

	SECTION("scanning a product with a special bogo set when the set amount of quantity is bought at the marked price increases the total by the price less percentage of the price denoted by the special") {
		testRegister.scanItem("fish");

		REQUIRE(testRegister.getQuantity("fish") == 1);
		REQUIRE(testRegister.getTotal() == 598);

		testRegister.scanItem("fish");

		REQUIRE(testRegister.getQuantity("fish") == 2);
		REQUIRE(testRegister.getTotal() == 598 + (int) (598 * .3 + .5));

		testRegister.scanItem("fish");

		REQUIRE(testRegister.getQuantity("fish") == 3);
		REQUIRE(testRegister.getTotal() == (598 * 2) + (int) (598 * .3 + .5));

		testRegister.scanItem("fish");

		REQUIRE(testRegister.getQuantity("fish") == 4);
		REQUIRE(testRegister.getTotal() == (598 * 2) + (2 * (int) (598 * .3 + .5)));
	}
	SECTION("scanning a product with a specialbogo set when the set quantity to purchase at full price is in the register results in the total being increased by the discounted amount for the amount set as the discountQuantity in the associated special object") {
		testRegister.scanItem("cereal");
		testRegister.scanItem("cereal");
		testRegister.scanItem("cereal");

		REQUIRE(testRegister.getQuantity("cereal") == 3);
		REQUIRE(testRegister.getTotal() == 299 * 3);

		testRegister.scanItem("cereal");
		testRegister.scanItem("cereal");

		REQUIRE(testRegister.getQuantity("cereal") == 5);
		REQUIRE(testRegister.getTotal() == 299 * 3);
		
		testRegister.scanItem("cereal");

		REQUIRE(testRegister.getQuantity("cereal") == 6);
		REQUIRE(testRegister.getTotal() == 299 * 4);
	}
	SECTION("removing an item with a set specialbogo that was added to the register at discount reduces the total by the discounted price") {
		testRegister.scanItem("cereal");
		testRegister.scanItem("cereal");
		testRegister.scanItem("cereal");
		testRegister.scanItem("cereal");
		testRegister.scanItem("cereal");

		REQUIRE(testRegister.getTotal() == 299 * 3);
		
		testRegister.removeItem("cereal");

		REQUIRE(testRegister.getTotal() == 299 * 3);

		testRegister.removeItem("cereal");

		REQUIRE(testRegister.getTotal() == 299 * 3);

		testRegister.removeItem("cereal");

		REQUIRE(testRegister.getTotal() == 299 * 2);
	}
	SECTION("scanning an item with a specialbogo with a quantity over the special limit does not trigger the special price") {
		testRegister.scanItem("cheese");
		testRegister.scanItem("cheese");
		testRegister.scanItem("cheese");
		testRegister.scanItem("cheese");

		REQUIRE(testRegister.getQuantity("cheese") == 4);
		REQUIRE(testRegister.getTotal() == 3 * 199);

		testRegister.scanItem("cheese");
		testRegister.scanItem("cheese");
		testRegister.scanItem("cheese");
		testRegister.scanItem("cheese");

		REQUIRE(testRegister.getQuantity("cheese") == 8);
		REQUIRE(testRegister.getTotal() == 7 * 199);
	}
	SECTION("removing a product with a specialbogo set with limit adjusts the total appropriately depending on whether a special is invalidated") {
		testRegister.scanItem("cheese");
		testRegister.scanItem("cheese");
		testRegister.scanItem("cheese");
		testRegister.scanItem("cheese");
		testRegister.scanItem("cheese");

		REQUIRE(testRegister.getTotal() == 4 * 199);

		testRegister.removeItem("cheese");

		REQUIRE(testRegister.getTotal() == 3 * 199);

		testRegister.removeItem("cheese");

		REQUIRE(testRegister.getTotal() == 3 * 199);

		testRegister.removeItem("cheese");

		REQUIRE(testRegister.getTotal() == 2 * 199);
	}
	SECTION("scanning a product with a specialbulk set adjusts the total to the noted special price when the set quantity of a product is a scanned") {
		testRegister.scanItem("coke");
		testRegister.scanItem("coke");
		testRegister.scanItem("coke");

		REQUIRE(testRegister.getQuantity("coke") == 3);
		REQUIRE(testRegister.getTotal() == 499 * 3);

		testRegister.scanItem("coke");

		REQUIRE(testRegister.getQuantity("coke") == 4);
		REQUIRE(testRegister.getTotal() == 1200);
	}
	SECTION("removing a product with a specialbulk set adjusts the total based on the special price if the change in quantity alters the qualifying special") {
		testRegister.scanItem("coke");
		testRegister.scanItem("coke");
		testRegister.scanItem("coke");
		testRegister.scanItem("coke");

		REQUIRE(testRegister.getQuantity("coke") == 4);
		REQUIRE(testRegister.getTotal() == 1200);

		testRegister.removeItem("coke");

		REQUIRE(testRegister.getQuantity("coke") == 3);
		REQUIRE(testRegister.getTotal() == 499 * 3);
	}
	SECTION("scanning an item with a special with a quantity over the limit value does not trigger the special bulk price") {
		testRegister.scanItem("bread");
		testRegister.scanItem("bread");
		testRegister.scanItem("bread");

		REQUIRE(testRegister.getQuantity("bread") == 3);
		REQUIRE(testRegister.getTotal() == 500);

		testRegister.scanItem("bread");
		testRegister.scanItem("bread");
		testRegister.scanItem("bread");

		REQUIRE(testRegister.getQuantity("bread") == 6);
		REQUIRE(testRegister.getTotal() == 500 + (235 * 3));
	}
	SECTION("removing an item with a specialbulk with a set limit adjusts the total based on whether a special is invalidated") {
		testRegister.scanItem("bread");
		testRegister.scanItem("bread");
		testRegister.scanItem("bread");
		testRegister.scanItem("bread");

		REQUIRE(testRegister.getTotal() == 735);

		testRegister.removeItem("bread");

		REQUIRE(testRegister.getTotal() == 500);

		testRegister.removeItem("bread");

		REQUIRE(testRegister.getTotal() == 470);
	}
	SECTION("scanning an item priced by weight correctly increases the total based on the special") {
		testRegister.scanItem("bacon", 300);

		REQUIRE(testRegister.getTotal() == 1750);

		testRegister.scanItem("bacon", 150);

		REQUIRE(testRegister.getTotal() == 1750 + ((int) ((150 / 100.0) * 700 + .5))); 
	}
	SECTION("removing an item priced by weight correctly decreases the total based on the special which may be invalidated") {
		testRegister.scanItem("bacon", 450);

		REQUIRE(testRegister.getTotal() == ((int) (700 * (350 / 100.0) + .5)) + ((int) (350 * (100 / 100.0) + .5)));

		testRegister.removeItem("bacon", 200);

		REQUIRE(testRegister.getTotal() == 1575);
	}
	SECTION("scanning an item priced by weight correctly increases the total based on the special and limit on the special") {
		testRegister.scanItem("shrimp", 600);

		REQUIRE(testRegister.getTotal() == ((int) (500 * (400 / 100.0) + .5)) + ((int) (500 * (200 / 100.0) * (25 / 100.0) + .5)));
	}
	SECTION("removing an item priced by weight correctly decreases the total based on the special and quantity limit which may be invalidated") {
		testRegister.scanItem("shrimp", 600);

		REQUIRE(testRegister.getTotal() == 2250);

		testRegister.removeItem("shrimp", 300);

		REQUIRE(testRegister.getTotal() == 1125);
	}
}
