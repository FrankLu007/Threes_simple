#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include <random>
#include "board.h"
#include "action.h"

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string property(const std::string& key) const { return meta.at(key); }
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};

class random_agent : public agent {
public:
	random_agent(const std::string& args = "") : agent(args) {
		if (meta.find("seed") != meta.end())
			engine.seed(int(meta["seed"]));
	}
	virtual ~random_agent() {}

protected:
	std::default_random_engine engine;
};

/**
 * random environment
 * add a new random tile to an empty cell
 * 2-tile: 90%
 * 4-tile: 10%
 */
class rndenv : public random_agent {
public:
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args),
		space({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }), last(-1),
		u({12, 13, 14, 15}), d({0, 1, 2, 3}), l({3, 7, 11, 15}), r({0, 4, 8, 12}) {bag.clear();}

	virtual action take_action(const board& after) {
		board::cell tile;
		if(bag.empty())
		{
			bag.push_back(3);
			bag.push_back(2);
			bag.push_back(1);
			std::shuffle(bag.begin(), bag.end(), engine);
		}
		tile = bag.back();
		bag.pop_back();
		if(last == -1) 
		{
			std::shuffle(space.begin(), space.end(), engine);
			for (int pos : space) {
				if (after(pos) != 0) continue;
				return action::place(pos, tile);
			}
		}
		else if(!last) 
		{
			std::shuffle(u.begin(), u.end(), engine);
			for (int pos : u) {
				if (after(pos) != 0) continue;
				return action::place(pos, tile);
			}
		}
		else if(last == 1) 
		{
			std::shuffle(r.begin(), r.end(), engine);
			for (int pos : r) {
				if (after(pos) != 0) continue;
				return action::place(pos, tile);
			}
		}
		else if(last == 2) 
		{
			std::shuffle(d.begin(), d.end(), engine);
			for (int pos : d) {
				if (after(pos) != 0) continue;
				return action::place(pos, tile);
			}
		}
		else 
		{
			std::shuffle(l.begin(), l.end(), engine);
			for (int pos : l) {
				if (after(pos) != 0) continue;
				return action::place(pos, tile);
			}
		}
		return action();
	}
	int last;
	std::vector<int> bag;
private:
	std::array<int, 16> space;
	std::array<int, 4> u, d, l, r;
};

/**
 * dummy player
 * select a legal action randomly
 */
class player : public random_agent {
public:
	player(const std::string& args = "") : random_agent("name=0516310 role=player " + args),
		opcode({0, 1, 2, 3}) {}

	virtual action take_action(const board& before) {
		//std::shuffle(opcode.begin(), opcode.end(), engine);
		int sol = 0;
		long long rew = -1;
		for (int op : opcode) {
			board::reward reward = board(before).slide(op);
			if (reward > rew) {sol = op; rew = reward;}
		}
		return action::slide(last = sol);
	}
	int last;
private:
	std::array<int, 4> opcode;
};
