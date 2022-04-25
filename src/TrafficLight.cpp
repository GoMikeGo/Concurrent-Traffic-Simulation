#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> rlck(_mtx);
    _cond.wait(rlck, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.front());
    _queue.clear();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> slck(_mtx);
    _queue.clear();
    _queue.push_back(std::move(msg));
    _cond.notify_one();
   
}


/* Implementation of class "TrafficLight" */

	 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _type = ObjectType::objectTrafficLight;
}


void TrafficLight::waitForGreen()
{

    while (true)
    {
        if(_msgQueue.receive() == TrafficLightPhase::green) 
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));   
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point t2;
    int phase_duration = 4000;
    int duration;
    while (true)
    {
        t2 = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
        if (duration >= phase_duration)
        {  
            t1 = t2;
            srand(time(0));
            phase_duration = rand() % 2000 + 4000;
            std::this_thread::sleep_for (std::chrono::milliseconds (phase_duration));
            _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;
            TrafficLightPhase message = _currentPhase;
            _msgQueue.send(std::move(message));  
        }
        std::this_thread::sleep_for (std::chrono::milliseconds (1));
    }
}



