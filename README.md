# MetaSounds Notify
MetaSounds meets the world!
Now, you can establish seamless communication between your MetaSounds systems and objects, actors, and blueprints within your game.

Have you ever encountered difficulties accessing variables within your MetaSounds systems? I've been there too! That's why I developed
this plugin, enabling you to send information from MetaSounds systems back to the world.

Enhance your gameplay experience by synchronizing events with music tracks or cue points, offering a more immersive gameplay journey.
With this plugin, you can easily send any desired information to the world, including cue points, strings, floats, integers, and booleans.

Enjoy complete compatibility with both Blueprints and C++, allowing for flexible integration into your game development workflow.

Personal use case:
I developed a spawner for the game "Ethernal" that synchronizes enemy waves with the music. Each time a wave concludes,
the next track loop seamlessly transitions in, and as each track loop begins, a new wave starts. Rather than abruptly
cutting the loops, we manipulate the Loop Start and Loop Duration parameters to ensure a smooth transition from one loop
to another.

However, this approach sometimes leads to a situation where a wave ends while the looped track portion is still playing.
Consequently, it becomes challenging to determine the precise moment to spawn the next wave in order to synchronize it 
with the music's beat drops. That was the case... until now.

With MetaSounds Notify, we have the solution. Whenever the track reaches a desired point, it sends an interface call
back to the spawner, which then spawns the next wave in perfect synchronization with the music.
