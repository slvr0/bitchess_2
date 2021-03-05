#include "mcts_node.h"

#include "math.h"

#include "utils/global_utils.cpp"
#include <ctime>
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace std::chrono;

#include <stdlib.h> // defines putenv in POSIX

#include <mutex>          // std::mutex

//std::mutex mtx;

using namespace mcts;


Node::Node(ChessBoard cb,Node* parent, int folder_id) :
    parent_(parent),
    cb_(cb),
    folder_id_(folder_id)
{
    if(parent) depth_ = parent->get_depth() + 1;
    else depth_ = 0;

    is_white_node_ = cb_.get_whitetoact();
}

Node::Node(ChessBoard cb, ChessMove move, Node* parent, int folder_id) :
    parent_(parent),
    cb_(cb),
    move_(move),
    folder_id_(folder_id)
{
    if(parent) depth_ = parent->get_depth() + 1;
    else depth_ = 0;

    is_white_node_ = cb_.get_whitetoact();
}


void Node::propagate_score_update(const float & score)
{
    //mtx.lock();

    if(abs(score) != 2.0f && is_white_node_ == 0) score_ -= score;
    else score_ += score;

    visits_++;

    //mtx.unlock();

    if(!logged_ && !this->is_leaf() && visits_ >= 50000)
    {
        bool log_criteria_fullfill = true;

//        for(const auto & child : this->get_childs() )
//        {
//            if(child->get_visits() <  10000)
//            {
//                log_criteria_fullfill = false;
//                break;
//            }
//        }

        if(log_criteria_fullfill)
        {
            logged_ = true;
            //dump data to a random file

            auto ms = duration_cast< milliseconds >(
                system_clock::now().time_since_epoch()
            );

            std::string full_path("training_data/sess_" + std::to_string(folder_id_) + "/file_" + std::to_string(ms.count()) + ".txt");

            std::ofstream fstream_(full_path);

           DataEncoder enc;
           this->nn_log_norecursive(enc, fstream_);
        }
    }

    if(parent_) parent_->propagate_score_update(score);
    else return;
}

double Node::ucb1_score()
{
    float explore_rate = 1.f;

    int parent_visits = parent_ ? parent_->get_visits() : 0;

    if(visits_ == 0) return is_white_node_ ? -100000.0 : 100000.0;
    else
    {
        if(is_white_node_) //if its white, its queued by black, hence it will want to be minimized
        {
            return (double) score_ / visits_  - explore_rate * std::sqrt(std::log(parent_visits) / visits_ );
        }
        else
        {
            return (double) score_ / visits_  + explore_rate * std::sqrt(std::log(parent_visits) / visits_ );
        }
    }
}

double Node::total_score() const
{
    if(visits_ == 0)
    {
        return score_;
    }
    else
    {
        return score_ / visits_;
    }
}

int Node::min_max() const
{
    double c_min =  10000.0;
    double c_max = -10000.0;

    int bindx = 0;

    for(int i = 0 ; i < childs_.size();++i)
    {
        double score = childs_.at(i)->ucb1_score();

        if(is_white_node_)
        {
            if(score>c_max)
            {
                c_max = score;
                bindx = i;
            }
        }
        else
        {
            if(score<c_min)
            {
                c_min = score;
                bindx = i;
            }
        }
    }
    return bindx;
}

void Node::debug_populate_ucbvec()
{
    std::vector<double> ucbvec;

    for(auto & child : childs_)
    {
        std::cout.precision(5);

        if(child->ucb1_score() == 1000000) std::cout << "inf ";
        else std::cout <<  child->ucb1_score() << " ";

        std::cout << "v : " << child->get_visits()<< " ";
        std::cout << "ts: " << child->total_score() << " ";

    }
    print("");
}

void Node::debug_print_childucb0()
{
    for(const auto & child : childs_)
    {
        if(child->ucb1_score() == 1000000) std::cout << "inf ";
        else  std::cout << child->ucb1_score() << " ";
    }
    std::cout << std::endl;
}

void Node::debug_print_child_totalscore()
{
    std::cout.precision(4);

    for(const auto & child : childs_)
    {
        //std::cout << "id: " << child->get_id() << " : ";

        child->get_move().print_move();

        if(child->get_visits() == 0) std::cout << 0 << " ";
        else std::cout << "visits : " << child->get_visits() << " score : " << child->total_score();

        std::cout << std::endl;
    }
}

//this will start  bottom nodes and traverse to root , each thread will print their bottom branches || skip root, skip bottom
void Node::nn_thread_log(DataEncoder& encoder_, std::ofstream & file)
{
    if(!parent_) // exit, dont log root
    {
        return;
    }
    else if(get_n_childs() ==0) //dont log bottom branches, but stil traverse up
    {
        parent_->nn_thread_log(encoder_, file);
    }


    MctsNodeData nn_data = encoder_.node_as_nn_input(*this);

    file << "new node -- depth : " <<  std::to_string(depth_) << "\n";
    file << "new node -- visits : " <<  std::to_string(visits_) << "\n";

    for(int i = 0 ; i < 13 ; ++i )
    {
        auto piece_vector = nn_data.bt_.get(i);

        auto piece_locations = bit_iter(piece_vector);

        for(const auto & loc : piece_locations)
        {
             file << int(loc) << " ";
        }

        file << "\n";
    }

    if(!nn_data.logits_.empty())
    {
        for(const auto & log : nn_data.logits_)
        {
            file << log << "  ";
        }

        file << "\n";

        for(const auto & log_idcs : nn_data.logits_idcs_)
        {
           file << log_idcs << "  ";
        }
    }
    else
    {
        file << "&\n&";
    }

    file << "\n";
    file << nn_data.value_;
    file << "\n";

    //get the n top and n bottom branch scores and send nn_log to them (later dude), right now just grab all input
    double tscores[get_n_childs()];

    for(int i = 0 ; i  < get_n_childs(); ++i)
    {
        auto score = get_child(i)->total_score();
        tscores[i] = score;
    }

    if(parent_)
    {
        parent_->nn_thread_log(encoder_, file);
    }
    else
    {
        return;
    }
}

void Node::nn_log_norecursive(DataEncoder &encoder_, std::ofstream &file)
{
    MctsNodeData nn_data = encoder_.node_as_nn_input(*this);

    file << "new node -- depth : " <<  std::to_string(depth_) << "\n";
    file << "new node -- visits : " <<  std::to_string(visits_) << "\n";

    for(int i = 0 ; i < 13 ; ++i )
    {
        auto piece_vector = nn_data.bt_.get(i);

        auto piece_locations = bit_iter(piece_vector);

        for(const auto & loc : piece_locations)
        {
             file << int(loc) << " ";
        }

        file << "\n";
    }

    if(!nn_data.logits_.empty())
    {
        for(const auto & log : nn_data.logits_)
        {
            file << log << "  ";
        }

        file << "\n";

        for(const auto & log_idcs : nn_data.logits_idcs_)
        {
           file << log_idcs << "  ";
        }
    }
    else
    {
        file << "&\n&";
    }

    file << "\n";
    file << nn_data.value_;
    file << "\n";

    //get the n top and n bottom branch scores and send nn_log to them (later dude), right now just grab all input,
    double tscores[get_n_childs()];

    for(int i = 0 ; i  < get_n_childs(); ++i)
    {
        auto score = get_child(i)->total_score();
        tscores[i] = score;
    }
}

void Node::nn_log(const int &n_top, const int &n_bottom, DataEncoder& encoder_, std::vector<std::ofstream*> vector_files, int entries_per_file, int &entry)
{
    auto file_idx = int(entry / (float)entries_per_file);

    if(file_idx >= vector_files.size())
    {
        print("searching file vector out of range");
        print(file_idx  );
        return;
    }

     ++entry;

    if(is_leaf())
    {
        return;
    }
    else
    {
        MctsNodeData nn_data = encoder_.node_as_nn_input(*this);

        *vector_files.at(file_idx) << "new node -- depth : " <<  std::to_string(depth_) << "\n";
        *vector_files.at(file_idx) << "new node -- visits : " <<  std::to_string(visits_) << "\n";

        for(int i = 0 ; i < 13 ; ++i )
        {
            auto piece_vector = nn_data.bt_.get(i);

            auto piece_locations = bit_iter(piece_vector);

            for(const auto & loc : piece_locations)
            {
                 *vector_files.at(file_idx) << int(loc) << " ";
            }

            *vector_files.at(file_idx) << "\n";
        }

        if(!nn_data.logits_.empty())
        {
            for(const auto & log : nn_data.logits_)
            {
                *vector_files.at(file_idx) << log << "  ";
            }

            *vector_files.at(file_idx) << "\n";

            for(const auto & log_idcs : nn_data.logits_idcs_)
            {
               *vector_files.at(file_idx) << log_idcs << "  ";
            }
        }
        else
        {
            *vector_files.at(file_idx) << "&\n&";
        }

        *vector_files.at(file_idx) << "\n";
        *vector_files.at(file_idx) << nn_data.value_;
        *vector_files.at(file_idx) << "\n";

        //get the n top and n bottom branch scores and send nn_log to them (later dude), right now just grab all input

        double tscores[get_n_childs()];

        for(int i = 0 ; i  < get_n_childs(); ++i)
        {
            auto score = get_child(i)->total_score();
            tscores[i] = score;
        }

        for(int i = 0 ; i  < get_n_childs(); ++i)
        {
            get_child(i)->nn_log(n_top, n_bottom, encoder_, vector_files, entries_per_file, entry);
        }
    }
}

int Node::color() const
{
    return is_white_node_;
}

void Node::setIs_white_node(int is_white_node)
{
    is_white_node_ = is_white_node;
}

std::pair<std::vector<float>, std::vector<int> > Node::get_logits_idc_pair() const
{
    std::vector<float> logits;
    std::vector<int> idc; // NN idcs!!

    DataEncoder encoder;

     MctsNodeData node_data = encoder.node_as_nn_input(*(this));

     return std::pair<std::vector<float>, std::vector<int> > (node_data.logits_, node_data.logits_idcs_);
}
