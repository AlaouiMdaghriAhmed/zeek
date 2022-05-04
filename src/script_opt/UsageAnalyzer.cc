// See the file "COPYING" in the main distribution directory for copyright.

#include "zeek/module_util.h"
#include "zeek/script_opt/IDOptInfo.h"
#include "zeek/script_opt/UsageAnalyzer.h"

namespace zeek::detail
	{

std::unordered_set<std::string> script_events;

void register_new_event(const IDPtr& id)
	{
	script_events.insert(id->Name());
	}

UsageAnalyzer::UsageAnalyzer(std::vector<FuncInfo>& funcs)
	{
	// Setting a scope cues ID::Traverse to delve into function values.
	current_scope = global_scope();

	FindSeeds(reachables);
	FullyExpandReachables();

	auto& globals = global_scope()->Vars();

	for ( auto& gpair : globals )
		{
		auto id = gpair.second.get();
		auto& t = id->GetType();

		if ( t->Tag() != TYPE_FUNC )
			continue;

		if ( t->AsFuncType()->Flavor() == FUNC_FLAVOR_FUNCTION )
			continue;

		if ( reachables.count(id) > 0 )
			continue;

		auto flavor = t->AsFuncType()->FlavorString();
		auto loc = id->GetLocationInfo();

		auto module = id->ModuleName();
		if ( module == GLOBAL_MODULE_NAME )
			module = "";
		else
			module += "::";

		reporter->Warning("%s %s%s (%s:%d): cannot be invoked", flavor.c_str(), module.c_str(), id->Name(), loc->filename, loc->first_line);

		reachables.insert(id);
		Expand(id);
		}

	for ( auto& gpair : globals )
		{
		auto& id = gpair.second;

		if ( reachables.count(id.get()) > 0 )
			continue;

		auto f = GetFuncIfAny(id);
		if ( ! f )
			continue;

		auto loc = id->GetLocationInfo();

		auto module = id->ModuleName();
		if ( module == GLOBAL_MODULE_NAME )
			module = "";
		else
			module += "::";

		reporter->Warning("function %s%s (%s:%d): cannot be called", module.c_str(), id->Name(), loc->filename, loc->first_line);
		}
	}

void UsageAnalyzer::FindSeeds(IDSet& seeds) const
	{
	for ( auto& gpair : global_scope()->Vars() )
		{
		auto& id = gpair.second;

		if ( id->GetAttr(ATTR_IS_USED) || id->GetAttr(ATTR_DEPRECATED) )
			{
			seeds.insert(id.get());
			continue;
			}

		auto f = GetFuncIfAny(id);

		if ( f && id->GetType<FuncType>()->Flavor() == FUNC_FLAVOR_EVENT )
			{
			if ( script_events.count(f->Name()) == 0 )
				seeds.insert(id.get());

			continue;
			}

		// If the global is exported, or has global scope, we assume
		// it's meant to be used, even if the current scripts don't
		// use it.
		if ( id->IsExport() || id->ModuleName() == "GLOBAL" )
			seeds.insert(id.get());
		}
	}

const Func* UsageAnalyzer::GetFuncIfAny(const ID* id) const
	{
	auto& t = id->GetType();
	if ( t->Tag() != TYPE_FUNC )
		return nullptr;

	auto fv = cast_intrusive<FuncVal>(id->GetVal());
	if ( ! fv )
		return nullptr;

	auto func = fv->Get();
	return func->GetKind() == Func::SCRIPT_FUNC ? func : nullptr;
	}

void UsageAnalyzer::FullyExpandReachables()
	{
	// We use the following structure to avoid having to copy
	// the initial set of reachables, which can be quite large.
	if ( ExpandReachables(reachables) )
		{
		auto r = new_reachables;
		reachables.insert(r.begin(), r.end());

		while ( ExpandReachables(r) )
			{
			r = new_reachables;
			reachables.insert(r.begin(), r.end());
			}
		}
	}

bool UsageAnalyzer::ExpandReachables(const IDSet& curr_r)
	{
	new_reachables.clear();

	for ( auto r : curr_r )
		Expand(r);

	return ! new_reachables.empty();
	}

void UsageAnalyzer::Expand(const ID* id)
	{
	analyzed_IDs.clear();
	id->Traverse(this);
	}

TraversalCode UsageAnalyzer::PreID(const ID* id)
	{
	if ( analyzed_IDs.count(id) > 0 )
		return TC_ABORTSTMT;

	analyzed_IDs.insert(id);

	auto f = GetFuncIfAny(id);

	if ( f && reachables.count(id) == 0 )
		new_reachables.insert(id);

	id->GetType()->Traverse(this);

	auto& attrs = id->GetAttrs();
	if ( attrs )
		attrs->Traverse(this);

	for ( auto& ie : id->GetOptInfo()->GetInitExprs() )
		if ( ie )
			ie->Traverse(this);

	return TC_CONTINUE;
	}

TraversalCode UsageAnalyzer::PreType(const Type* t)
	{
	if ( analyzed_types.count(t) > 0 )
		return TC_ABORTSTMT;

	analyzed_types.insert(t);
	return TC_CONTINUE;
	}

	} // namespace zeek::detail
