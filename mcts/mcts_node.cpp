#include "mcts_node.h"

#include "math.h"

#include "utils/global_utils.cpp"


using namespace mcts;


Node::Node(ChessBoard cb,Node* parent, int id) :
    parent_(parent),
    cb_(cb),
    id_(id)
{
    if(parent) depth_ = parent->get_depth() + 1;
    else depth_ = 0;

    is_white_node_ = cb_.get_whitetoact();
}

Node::Node(ChessBoard cb, ChessMove move, Node* parent, int id) :
    parent_(parent),
    cb_(cb),
    move_(move),
    id_(id)
{
    if(parent) depth_ = parent->get_depth() + 1;
    else depth_ = 0;

    is_white_node_ = cb_.get_whitetoact();
}

void Node::propagate_score_update() //on rollout
{
    visits_++;

    if(!parent_) return ;

    switch(status_)
    {
        case 0 : score_ += 0.f ; parent_->propagate_score_update(0);break;
        case 1 : score_ += 1.f; parent_->propagate_score_update(1.f);break;
        case 2 : score_ -=  1.f;parent_->propagate_score_update(-1.f);break;
        case 3 :
        {
            visits_ += 10; //haxx it so its unlikely to go here again
            score_ = is_white_node_ ? score_ + .5f : score_ -.5f;
            parent_->propagate_score_update(.5f);
            break;
        }
        case 4 :
        {
            visits_ += 10; //haxx it so its unlikely to go here again
            score_ = is_white_node_ ? score_ + .5f : score_ -.5f;
            parent_->propagate_score_update(.5f);
            break;
        }
    }
}

void Node::propagate_score_update(const float & score) //on parent of rollout
{
    visits_++;

    if(score == .5f)
    {
        score_ = is_white_node_ ? score_ + .5f : score_ -.5f;
    }
    else
    {
        score_ += score;
    }

    if(!parent_)
    {
        return;
    }

    parent_->propagate_score_update(score);
}

double Node::ucb1_score()
{
    float explore_rate = 2.f;

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

double Node::total_score()
{
    if(visits_ == 0)
    {
        return score_;
    }
    else
    {
        return  (double)score_ / visits_;
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
        else std::cout << child->total_score();

        std::cout << std::endl;
    }
}

void Node::nn_log(const int &n_top, const int &n_bottom, DataEncoder& encoder_, std::ofstream &node_data)
{
    auto argmax = [this](double* dlist, int N)
    {
        double m_val = -1000.f;
        int m_idx  = 0;

        for(int i = 0 ; i < N ; ++i)
        {
            if(dlist[i] > m_val)
            {
                m_val = dlist[i];
                m_idx = i;
            }
        }
        return m_idx;
    };

    auto argmin = [this](double* dlist,  int N)
    {
        double m_val = 1000.f;
        int m_idx  = 0;
        for(int i = 0 ; i < N ; ++i)
        {
            if(dlist[i] < m_val)
            {
                m_val = dlist[i];
                m_idx = i;
            }
        }
        return m_idx;
    };

    MctsNodeData nn_data = encoder_.node_as_nn_input(this);

    for(int i = 0 ; i < 13 ; ++i )
    {
        auto piece_vector = nn_data.bt_.get(i);

        auto piece_locations = bit_iter(piece_vector);

        for(const auto & loc : piece_locations)
        {
             node_data << int(loc) << " ";
        }

        node_data << "\n";
    }

    if(!nn_data.logits_.empty())
    {
        for(const auto & log : nn_data.logits_)
        {
            node_data << log << "  ";
        }

        node_data << "\n";

        for(const auto & log_idcs : nn_data.logits_idcs_)
        {
            node_data << log_idcs << "  ";
        }
    }
    else
    {
        node_data << "&\n&";
    }

    node_data << "\n";
    node_data << nn_data.value_;
    node_data << "\n";

    if(is_leaf())
    {
        return;
    }
    else
    {
        //get the n top and n bottom branch scores and send nn_log to them (later dude), right now just grab all input

        double tscores[get_n_childs()];

        for(int i = 0 ; i  < get_n_childs(); ++i)
        {
            auto score = get_child(i)->total_score();
            tscores[i] = score;
        }

        for(int i = 0 ; i  < get_n_childs(); ++i)
        {
            get_child(i)->nn_log(n_top, n_bottom, encoder_, node_data);
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
