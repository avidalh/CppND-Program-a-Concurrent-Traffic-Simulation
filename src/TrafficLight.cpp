#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable    
    std::cout << "Rx ()....." << std::endl;

    T msg = std::move(_queue.back());
    std::cout << "   Rx Message " << msg << " has been sent to the queue" << std::endl;

    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::lock_guard<std::mutex> uLock(_mutex);    
    // std::cout << "   Message " << msg << " has been sent to the queue" << std::endl;
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
    while (true)
    {
        TrafficLightPhase isGreen;
        MessageQueue<TrafficLightPhase> message;
        if (message.receive() == TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return TrafficLight::_currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when
    //the public method „simulate“ is called. To do this, use the thread queue in the base class.

    std::shared_ptr<MessageQueue<int>> queue(new MessageQueue<int>);
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));  //std::async(std::launch::async, &MessageQueue<int>::send, queue, std::move(message)))
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    MessageQueue<TrafficLightPhase> message;
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    lastUpdate = std::chrono::system_clock::now();
    while (true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        int cycleDuration = rand() % 2000 + 4000; //pseudo random number between 4 and 6 secs.
        if (timeSinceLastUpdate >= cycleDuration){
            lastUpdate = std::chrono::system_clock::now();
            if(_currentPhase == TrafficLightPhase::red){
                // std::cout << "passing to green" << std::endl;
                _currentPhase = TrafficLightPhase::green;
            }
            else{
                // std::cout << "passing to red" << std::endl;
                _currentPhase = TrafficLightPhase::red;
            }
            message.send(std::move(_currentPhase));
        }
    }
}
