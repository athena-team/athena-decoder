import sys

if len(sys.argv)!=3:
    print("usages: remove items in lexicon that are not in graphemes");
    print("usages: filter_lexicon.py lexicon.txt graphemes.txt");
    exit(1)

graphemes = set()
for line in open(sys.argv[2]):
    items = line.strip().split()
    graphemes.add(items[0])

for line in open(sys.argv[1]):
    items = line.strip().split()
    for item in items[1:]:
        if item not in graphemes:
            break
        else:
            print("%s"%(' '.join(items)))

