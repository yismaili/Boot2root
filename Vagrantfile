# -*- mode: ruby -*-

# vi: set ft=ruby :

Vagrant.configure("2") do |config|

    config.vm.box = "kalilinux/rolling"
  
    config.vm.box_check_update = false
  
    config.vm.hostname = "kali-linux"
  
    config.vm.provider "virtualbox" do |vb|
        vb.gui = false
        vb.memory = "2048"
    end
    config.vm.network "public_network", bridge: "en0"

end
