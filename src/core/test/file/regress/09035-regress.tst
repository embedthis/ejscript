/*
    Test reading a file without a '\n' on the last line
 */

require ejs.unix

fname = "/tmp/" + hashcode(self) + ".tmp"

data = "example text example text    001 
example text example text    002 
example text example text    003 
example text example text    004 
example text example text    005 
example text example text    006 
example text example text    007 
example text example text    008 
example text example text    009 
example text example text    010 
example text example text    011 
example text example text    012 
example text example text    013 
example text example text    014 
example text example text    015 
example text example text    016 
example text example text    017 
example text example text    018 
example text example text    019 
example text example text    020 
example text example text    021 
example text example text    022 
example text example text    023 
example text example text    024 
example text example text    025 
example text example text    026 
example text example text    027 
example text example text    028 
example text example text    029 
example text example text    030 
example text example text    031 
example text example text    032 
example text example text    033"


// Create example text file
file = new File(fname)
file.open("w")
file.write(data)
file.close()

// Check for the bug
var s: TextStream = Path(fname).openTextStream("rt")
data2 = ""
while (line = s.readLine())
    data2 += line + "\n"
s.close()
rm(fname)
