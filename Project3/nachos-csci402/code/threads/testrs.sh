#! bin/bash
#! /bin/csh -f
set c=1
while ( $c <= 10 )
echo "nachos -P2 -rs $c >> out.txt"
echo "nachos -P2 -rs $c >> out.txt" >> out.txt
    `nachos -P2 -rs $c | grep -i "leaves the Doctor" | wc -l >> out.txt`
     set c = $c + 1
end
