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
    _queue.emplace_back(std::move(msg));
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
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4000, 6000);
    int phase_duration = distr(eng);
    std::chrono::high_resolution_clock::time_point lastUpdate = std::chrono::high_resolution_clock::now();
    long timeSinceLastUpdate; 
    TrafficLightPhase message; 
  while (true)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (1));
        timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= phase_duration)
        {  
            _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;
            message = _currentPhase;
            _msgQueue.send(std::move(message));
            lastUpdate = std::chrono::high_resolution_clock::now();
            phase_duration = distr(eng);
        }
    }
}



