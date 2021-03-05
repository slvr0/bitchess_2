#include "rollout_test.h"

#include <random>
#include "utils/global_utils.cpp"

RolloutPhaseTest::RolloutPhaseTest()
{
    srand((unsigned) time(0));
}

float RolloutPhaseTest::test_rollouts(std::string position, const int &n_simuls, const int &n_steps)
{
    ChessBoard cb(position);

    float simul_avg_score = 0;
    int n_draws = 0;
    int n_decisive = 0;

    for(int simul = 0 ; simul < n_simuls; ++simul)
    {
        float score = 0;
        env_.reset(cb);

        for(int i = 0 ; i < n_steps; ++i)
        {
            BoardInfo info;

            auto moves = env_.get_actions();

            info = env_.get_info();

            score = info.reward;

            if(info.done )
            {
            break;
            }

            auto rm = rand() % moves.size();

            env_.step(rm);
        }
        if(abs(score) == 1.0f)
        {
            simul_avg_score += score;
            n_decisive++;
        }
        else
        {
            n_draws++;
        }
    }

    cb.print_to_console();

    std::cout << "average score after " << n_simuls << " number of decisive outcomes : " << n_decisive << " simulations, _AVG_ score : " << simul_avg_score << " position was drawn : " << n_draws << " times" <<  std::endl;
    return simul_avg_score;
}
