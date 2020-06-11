#include "../Game_local.h"

#include "../CmdSystem.h"

/*
===================
Cmd_EntityList_f
===================
*/
void Cmd_EntityList_f(/*const idCmdArgs& args*/) {
	int e;
	int count;
	size_t size;
	std::string match;

	/*if (args.Argc() > 1) {
		match = args.Args();
		match.Replace(" ", "");
	}
	else {
		match = "";
	}*/

	count = 0;
	size = 0;

	gameLocal.Printf("%-4s  %-30s %-20s %s\n", " Num", "EntityDef", "Class", "pos");
	gameLocal.Printf("--------------------------------------------------------------------\n");
	for (e = 0; e < MAX_GENTITIES; e++) {
		auto check = gameLocal.entities[e];

		if (!check) {
			continue;
		}

		/*if (!check->name.Filter(match, true)) {
			continue;
		}*/

		gameLocal.Printf("%4i: %-30s %-20s [%5.2f %5.2f]\n", e,
			check->GetName().c_str(), check->GetClassname().c_str(), check->GetPhysics()->GetOrigin().x,
			check->GetPhysics()->GetOrigin().y);

		count++;
		//size += check->spawnArgs.Allocated();
	}

	gameLocal.Printf("...%d entities\t...%d bytes of spawnargs\n", count, size);
}