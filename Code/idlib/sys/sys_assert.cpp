#include "precompiled.h"

struct skippedidassertion_t {
	skippedidassertion_t() {
	}

	std::string file = "";
	int				line = -1;
};
static std::vector<skippedidassertion_t> skippedidassertions;

bool idassertFailed(const std::string& file, int line, const std::string& expression) {
	// Set this to true to skip ALL idassertions, including ones YOU CAUSE!
	static volatile bool skipAllidassertions = false;
	if (skipAllidassertions) {
		return false;
	}

	// Set this to true to skip ONLY this idassertion
	static volatile bool skipThisidassertion = false;
	skipThisidassertion = false;

	for (size_t i = 0; i < skippedidassertions.size(); i++) {
		if (skippedidassertions[i].file == file && skippedidassertions[i].line == line) {
			skipThisidassertion = true;
			// Set breakpoint here to re-enable
			if (!skipThisidassertion) {
			}
			return false;
		}
	}

	idLib::Warning("idassertION FAILED! %s(%d): '%s'", file.c_str(), line, expression.c_str());

	if (IsDebuggerPresent()) {
		__debugbreak();
	}

	if (skipThisidassertion) {
		skippedidassertion_t skipped;
		skipped.file = file;
		skipped.line = line;

		skippedidassertions.push_back(skipped);
	}

	return true;
}
