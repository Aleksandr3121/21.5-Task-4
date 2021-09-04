#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <limits>

using namespace std;

struct Coordinate {
    int x, y;
};

Coordinate operator+(const Coordinate& rhs, const Coordinate& lhs) {
    Coordinate result = {rhs.x + lhs.x, rhs.y + lhs.y};
    if (result.x > 39)result.x = 39;
    if (result.x < 0)result.x = 0;
    if (result.y > 39)result.y = 39;
    if (result.y < 0)result.y = 0;
    return result;
}

bool operator==(const Coordinate& rhs, const Coordinate& lhs) {
    return rhs.x == lhs.x && rhs.y == lhs.y;
}

bool operator!=(const Coordinate& rhs, const Coordinate& lhs) {
    return !(rhs==lhs);
}

istream& operator>>(istream& stream, Coordinate& coordinate) {
    stream >> coordinate.x >> coordinate.y;
    if (coordinate.x < 0 || coordinate.x > 39 || coordinate.y < 0 || coordinate.y > 39) {
        stream.clear(stream.rdstate() | ios::failbit);
    }
    return stream;
}

struct Character {
    string name;
    int hp = 0;
    int armor = 0;
    int damage = 0;
    Coordinate coordinate;
    bool enemy = true;
    bool in_game = true;

    int Damage(Character& foe) {
        int hp_foe_start = foe.hp;
        if (damage >= (foe.armor + foe.hp)) {
            foe.hp = 0;
            foe.armor = 0;
            foe.in_game = false;
        } else if (damage >= foe.armor) {
            foe.hp -= (damage - foe.armor);
            foe.armor = 0;
        } else {
            foe.armor -= damage;
        }
        cout << name << " has done damage " << hp_foe_start - foe.hp << " -HP" << endl;
        return hp_foe_start - foe.hp;
    }
};


void PreparingThePlayingField(char field[][40], vector<Character>& people) {
    srand(time(0));
    for (int i = 0; i < 40; ++i) {
        for (int j = 0; j < 40; ++j) {
            field[i][j] = '.';
        }
    }
    cout << "Enter name of person: ";
    cin >> people[0].name;
    cout << "Enter HP of person: ";
    cin >> people[0].hp;
    cout << "Enter armor of person: ";
    cin >> people[0].armor;
    cout << "Enter damage of person: ";
    cin >> people[0].damage;
    cout << "Enter coordinate of person x and y: ";
    while (!(cin >> people[0].coordinate)) {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.clear();
        cout << "Error of input" << endl;
        cout << "Enter coordinate of person x and y: ";
    }
    people[0].enemy = false;
    for (int i = 1; i < people.size(); ++i) {
        people[i].name = ("Enemy #" + to_string(i));
        people[i].hp = rand() % 101 + 50;
        people[i].armor = rand() % 51;
        people[i].damage = rand() % 16 + 15;
        bool not_correct_coordinate = true;
        while (not_correct_coordinate) {
            not_correct_coordinate = false;
            people[i].coordinate = {rand() % 40, rand() % 40};
            for (int j = 0; j < i; ++j) {
                if (people[j].coordinate != people[i].coordinate) continue;
                else {
                    not_correct_coordinate = true;
                    break;
                }
            }
        }
    }
    for (const auto& person: people) {
        if (person.enemy) field[person.coordinate.y][person.coordinate.x] = 'E';
        else field[person.coordinate.y][person.coordinate.x] = 'P';
    }
}

bool PrintField(char field[][40], const vector<Character>& people) {
    for (int i = 0; i < 40; ++i) {
        for (int j = 0; j < 40; ++j) {
            cout << field[i][j];
        }
        cout << endl;
    }
    for(const auto& person : people){
        clog<<person.name<<" HP "<<person.hp<<" Armor "<<person.armor<<endl;
    }
    return true;
}

void Save(const vector<Character>& people) {
    ofstream file("save.bin", ios::binary);
    if (!file.is_open())throw invalid_argument("");
    for (const Character& person : people) {
        int size_name = person.name.size();
        file.write((char*) &size_name, sizeof(int));
        file.write(person.name.c_str(), size_name);
        file.write((char*) &person.hp, sizeof(int));
        file.write((char*) &person.armor, sizeof(int));
        file.write((char*) &person.damage, sizeof(int));
        file.write((char*) &person.coordinate.x, sizeof(int));
        file.write((char*) &person.coordinate.y, sizeof(int));
        file.write((char*) &person.enemy, sizeof(bool));
        file.write((char*) &person.in_game, sizeof(bool));
    }
    file.close();
}

void Load(char field[][40], vector<Character>& people) {
    ifstream file("save.bin", ios::binary);
    if (!file.is_open()) {
        cerr << "Not saving" << endl;
        return;
    }
    for (int i = 0; i < 40; ++i) {
        for (int j = 0; j < 40; ++j) field[i][j] = '.';
    }
    for (Character& person: people) {
        int size_name;
        file.read((char*) &size_name, sizeof(int));
        person.name.resize(size_name);
        file.read((char*) person.name.c_str(), size_name);
        file.read((char*) &person.hp, sizeof(int));
        file.read((char*) &person.armor, sizeof(int));
        file.read((char*) &person.damage, sizeof(int));
        file.read((char*) &person.coordinate.x, sizeof(int));
        file.read((char*) &person.coordinate.y, sizeof(int));
        file.read((char*) &person.enemy, sizeof(bool));
        file.read((char*) &person.in_game, sizeof(bool));
        if (person.enemy && person.in_game) field[person.coordinate.y][person.coordinate.x] = 'E';
        else field[person.coordinate.y][person.coordinate.x] = 'P';
    }
    file.close();
}

enum Commands {
    left,
    right,
    top,
    bottom
};

const vector<Coordinate> direction_vectors = {{-1, 0},
                                              {1,  0},
                                              {0,  -1},
                                              {0,  1}};

Commands StringToCommands(const string& str) {
    if (str == "left")return Commands::left;
    else if (str == "right")return Commands::right;
    else if (str == "top")return Commands::top;
    else if (str == "bottom")return Commands::bottom;
    else throw invalid_argument("");
}

int main() {
    char field[40][40];
    vector<Character> people(6);
    PreparingThePlayingField(field, people);
    int hp_enemies = 0;
    for (int i = 1; i < people.size(); ++i) hp_enemies += people[i].hp;
    string command;
    size_t index_command;
    bool step_of_player;
    while (PrintField(field, people) && people[0].hp > 0 && hp_enemies > 0) {
        //PrintField(field, people);
        step_of_player = true;
        for (int i = 0; i < people.size(); ++i) {
            if (!people[i].in_game)continue;
            if (step_of_player && !people[i].enemy) {
                cin >> command;
                if (command == "save") {
                    Save(people);
                    break;
                } else if (command == "load") {
                    Load(field, people);
                    break;
                }
                index_command = StringToCommands(command);
                step_of_player = false;
            } else index_command = rand() % direction_vectors.size();
            Coordinate movement = people[i].coordinate + direction_vectors[index_command];
            if (field[movement.y][movement.x] == '.') {
                field[people[i].coordinate.y][people[i].coordinate.x] = '.';
                if (people[i].enemy) field[movement.y][movement.x] = 'E';
                else field[movement.y][movement.x] = 'P';
                people[i].coordinate = movement;
            } else if (people[i].coordinate == movement) {
                //skip
            } else if ((people[i].enemy && field[movement.y][movement.x] == 'P')
                       || (!people[i].enemy && field[movement.y][movement.x] == 'E')) {
                size_t index_foe = 0;
                while (people[index_foe].coordinate != movement || !people[index_foe].in_game) {
                    index_foe++;
                }
                hp_enemies -= people[i].Damage(people[index_foe]);
                if (!people[index_foe].in_game) {
                    field[people[i].coordinate.y][people[i].coordinate.x] = '.';
                    if (people[i].enemy) field[movement.y][movement.x] = 'E';
                    else field[movement.y][movement.x] = 'P';
                    people[i].coordinate = movement;
                }
            }
        }
    }
    if (people[0].in_game)cout << "Victory!" << endl;
    else cout << "Defeat!" << endl;
    return 0;
}
