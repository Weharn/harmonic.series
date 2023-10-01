harmonic.series

This is a program that will find the harmonic series up to the 15th over- or under- tone. It will take the note in standard notation (i.e. G#6, A4, Eb3) and find the harmonic series in terms of frequencies. 

Then, depending on a specified parameter (merge or add) it will write a .wav file of the harmonic series. The add paramater will mean the notes are simply added sequentially; the merge paramater will mean that the notes are continually stacked on top of one another as a chord.

The seq_series process. This process simply creates and instantly writes a sequential harmonic series to file.

The lay_series process. This process creates a layered harmonic series. For every new harmonic added (except for the fundamental and first over/under -tone), the process creates a vector of Sine objects. It then initialises these Sines. Then, for every "step" (or sample), the process finds the new value of the sine waves, and adds them (multiplying by the reciprocal of how many there are first to ensure that they are properly balanced and do not make the final sample overflow) to a sum variable. This is the combined sample. That "sum" sample is the appropriately expanded with amplmod, casted to int, and written to file.

The run() function packages everything up and does all the work to connect the other aspects of the program. It should work regardless of the machine or directory on or in which the program (and frequencies.txt) are located.

However, inexplicably, when the program is built or debugged on the "Release" compiler configuraion in Visual Studio 2022, the ofstream never opens. It always works when the "Debug" configuration is used.
