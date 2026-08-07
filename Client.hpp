/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgorceac <dgorceac@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/07 12:54:41 by dgorceac          #+#    #+#             */
/*   Updated: 2026/08/07 14:13:19 by dgorceac         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
    private:
        int fd;
        std::string nickname;
        std::string username;
        std::string buffer;
    
    public:
        Client();
        Client(int fd);
        Client(const Client& other);
        Client& operator=(const Client& other);
        ~Client();

        int getFd() const;
};

#endif