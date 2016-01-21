//
//  AppDelegate.swift
//  doorbell
//
//  Created by 周竞 on 16/1/21.
//  Copyright © 2016年 Lex Chou. All rights reserved.
//

import Cocoa
import Foundation

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate, NSStreamDelegate {
    let serverAddress: CFString = "10.0.0.1"
    let serverPort: UInt32 = 3001
    var trayItem : NSStatusItem?;
    var inputStream : NSInputStream?
    var outputStream : NSOutputStream?

    func quit(sender : NSObject) {
        NSApp.terminate(sender);
    }
    func applicationDidFinishLaunching(aNotification: NSNotification) {
        // Insert code here to initialize your application
        let menu = NSMenu.init();
        let item = menu.addItemWithTitle("Quit", action: Selector("quit:"), keyEquivalent: "");
        item?.target = self;
        trayItem = NSStatusBar.systemStatusBar().statusItemWithLength(NSSquareStatusItemLength)
        trayItem?.menu = menu;
        trayItem?.highlightMode = true;
        trayItem?.title = "铃";
        
        connect();
        
    }
    func disconnect() {
        guard let inputStream = self.inputStream else {return}
        guard let outputStream = self.outputStream else {return}

        inputStream.removeFromRunLoop(NSRunLoop.currentRunLoop(), forMode: NSDefaultRunLoopMode);
        inputStream.close();
        outputStream.removeFromRunLoop(NSRunLoop.currentRunLoop(), forMode: NSDefaultRunLoopMode);
        outputStream.close();
        self.inputStream = nil
        self.outputStream = nil
    }
    func connect() {
        print("Connecting to \(serverAddress):\(serverPort)");
        var readStream:  Unmanaged<CFReadStream>?
        var writeStream: Unmanaged<CFWriteStream>?
        
        CFStreamCreatePairWithSocketToHost(nil, self.serverAddress, self.serverPort, &readStream, &writeStream)
        
        self.inputStream = readStream!.takeRetainedValue()
        self.outputStream = writeStream!.takeRetainedValue()
        guard let inputStream = self.inputStream else {return}
        guard let outputStream = self.outputStream else {return}
        
        inputStream.delegate = self
        outputStream.delegate = self
        
        inputStream.scheduleInRunLoop(NSRunLoop.currentRunLoop(), forMode: NSDefaultRunLoopMode)
        outputStream.scheduleInRunLoop(NSRunLoop.currentRunLoop(), forMode: NSDefaultRunLoopMode)
        
        inputStream.open()
        outputStream.open()
    }
    func stream(stream: NSStream, handleEvent eventCode: NSStreamEvent) {
        switch(eventCode)
        {
        case NSStreamEvent.ErrorOccurred:
            print("Error occurred, close the connection and try again in next 5 seconds...");
            disconnect();
            NSTimer.scheduledTimerWithTimeInterval(NSTimeInterval(5), target: self, selector: Selector("connect"), userInfo: nil, repeats: false);
            break;
        case NSStreamEvent.OpenCompleted:
            print("Open completed");
            break;
        case NSStreamEvent.HasBytesAvailable:
            print("bytes available")
            if let inputStream = self.inputStream {
                
                var buf = [UInt8](count : 20, repeatedValue : 0)
                let n = inputStream.read(&buf, maxLength: buf.count);
                if var content = NSString.init(bytes: &buf, length: n, encoding: NSUTF8StringEncoding)
                {
                    content = content.stringByTrimmingCharactersInSet(
                        NSCharacterSet.whitespaceAndNewlineCharacterSet())
                    if content == "RING" {
                        let notif = NSUserNotification()
                        notif.title = "门铃"
                        notif.informativeText = "有人在敲门"
                        notif.soundName = NSUserNotificationDefaultSoundName
                        NSUserNotificationCenter.defaultUserNotificationCenter().deliverNotification(notif)
                    }
                    print(content)
                }
            }
            break;
        case NSStreamEvent.EndEncountered:
            print("ended");
            break;
        default:
            print("Other event");
            break;
        }
    }

    func applicationWillTerminate(aNotification: NSNotification) {
        // Insert code here to tear down your application
    }


}

