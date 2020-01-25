#include <iostream>
#include <random>
#include <ctime>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> ulock(_mtx);
    _cond.wait(ulock, [this]{return !_queue.empty();});

    // wait (block this main thread), unlock mutex so that other threads can add msgs,
    // when get notified, lock mutex, check if it was a spurious wakeup or actual notification by checking if msgsQueue not empty,
    // if actual notification, wake up from wait and continue pop back.

    T msg = std::move(_queue.front());
    _queue.pop_front();
    return msg; 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mtx);

    //std::cout << "   Message " << int(msg) << " has been sent to the queue" << std::endl;
    _queue.push_back(std::move(msg));
    _cond.notify_one();

}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        TrafficLightPhase newLight=_msgQueue.receive();
        if (newLight == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    //std::lock_guard<std::mutex> lckCurrentPhase(_mutex);
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    srand(time(NULL));
    double cycleDuration = rand()%(6-4+1)+4; // Generate the number, assign to variable.; 
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    lastUpdate = std::chrono::system_clock::now();

    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        uint timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
            //std::unique_lock<std::mutex> lckCurrentPhase(_mutex);
            if (_currentPhase == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;
            }
            else
            {
                _currentPhase = TrafficLightPhase::red;
            }

            _msgQueue.send(std::move(_currentPhase));
            //lckCurrentPhase.unlock();

            // reset clock
            lastUpdate = std::chrono::system_clock::now();
        }


    }
}

