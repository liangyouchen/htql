import htql
import re

def test_basic_regex(tests):
    b='';
    for i, test in enumerate(tests):
        print(i); 
        a=htql.RegEx(); 
        useindex=False;
        if 'useindex' in test.keys(): useindex=test['useindex'];
        case=False;
        if 'case' in test.keys(): case=test['case'];
        overlap=False;
        if 'overlap' in test.keys(): overlap=test['overlap'];
        group=False;
        if 'group' in test.keys(): group=test['group']; 
        if 'var' in test.keys(): 
            for n,v in test['var'].items(): 
                a.setNameSet(n,v);
        if isinstance(test['sent'], str): 
            b=a.reSearchStr(test['sent'], test['reg'], case=case, overlap=overlap, useindex=useindex, group=group);
        else:
            b=a.reSearchList(test['sent'], test['reg'], case=case, overlap=overlap, useindex=useindex, group=group);
        if re.sub(r'(\d)L', r'\1', repr(b))!=re.sub(r'(\d)L', r'\1', repr(test['result'])):
            print("Error %d: " % i);
            print(repr([test]));
            if 'var' in test.keys():
                for n,v in test['var'].items(): 
                    print("a.setNameSet(%s,%s)" % (
                        repr(n), repr(v), 
                        ));
            print("a.reSearchList(%s,%s,case=%s,overlap=%s,useindex=%s);" % (
                repr(test['sent']), repr(test['reg']), repr(case), repr(overlap), repr(useindex),
                ));
            print("  ..Returned: %s\r\n..Expecting: %s\r\n\r\n" % (
                repr(b), repr(test['result']),
                ));


tests=[
        {'sent':'abcba', 'reg':'[ab]+', 'result':['ab', 'ba'] },
        {'sent':'aba', 'reg':'a', 'result':['a', 'a'] },
        {'sent':'aba', 'reg':'(ab)', 'result':['ab'] },
        {'sent':'abba', 'reg':'^a', 'result':['a'] },
        {'sent':'abba', 'reg':'ab+', 'result':['abb'] },
        {'sent':'abba', 'reg':'a$', 'result':['a'] },
        {'sent':'abba', 'reg':'a.{0,2}a', 'result':['abba'] },
        {'sent':'abba', 'reg':'a.{1}a', 'result':[] },
        {'sent':'abba', 'reg':'a.{3}a', 'result':[] },
        {'sent':'abbaaca', 'reg':'[^a]+', 'result':['bb', 'c'] },
        {'sent':'abbaaca', 'reg':'[^a]{2}', 'result':['bb'] },
        {'sent':'abbaacaba', 'reg':'(ba|ac)+', 'result':['baac', 'ba'] },
        {'sent':'abbaacaba', 'reg':'[^(ba|ac)]+', 'result':['abbaacaba'] },
        {'sent':'thxxes', 'reg':'th[^(ab|cd)]*es', 'result':['thxxes'] },
        {'sent':'useful', 'reg':'.*e.*', 'result':['useful'] },
        {'sent':'b', 'reg':'b.*', 'result':['b'] },
        {'sent':'b', 'reg':'b.+', 'result':[] },
        {'sent':'A', 'reg':'[ND]+', 'result':[] },
        {'sent':'NN', 'reg':'[ND]+', 'result':['NN'] },
        {'sent':['NN'], 'reg':'[<CD>]+', 'result':[] },
        {'sent':['NN', 'NN'], 'reg':'[<VB.*><CD>]+', 'result':[] },
        {'sent':['NN', 'NN'], 'reg':'[<NN.*><CD>]+', 'result':[['NN', 'NN']] },
        {'sent':[('is', 'VBZ'), ], 'reg':'[<.*/NN.*><.*/CD>]+', 'result':[] },
        {'sent':'abc', 'reg':'(.*)c', 'result':['abc'] },
        {'sent':'abc', 'reg':'.*c', 'result':['abc'] },
        {'sent':'ac', 'reg':'ad*', 'result':['a'] },
        {'sent':'ac', 'reg':'ad*c', 'result':['ac'] },
        {'sent':'ac', 'reg':'ad{0,1}c', 'result':['ac'] },
        {'sent':'ac', 'reg':'ad{0,0}c', 'result':['ac'] },
        {'sent':'ac', 'reg':'ad{0}c', 'result':['ac'] },
        {'sent':'ac', 'reg':'ad*ce*', 'result':['ac'] },
        {'sent':'ac', 'reg':'ad*ce*f*', 'result':['ac'] },
        {'sent':'adc', 'reg':'ad{0}c', 'result':[] },
        {'sent':['t'], 'reg':'<t>.*', 'result':[['t']] },
        {'sent':[('t','N')], 'reg':'<t/.*>.*', 'result':[[('t', 'N')]] },
        {'sent':'b12vc', 'reg':r'\d*', 'result':['', '12', '', ''] },
        {'sent':'useful', 'reg':'[sEf]{2}', 'result':['se'] },
        {'sent':[('my', 'PRP'),('test', 'NN')], 
                    'reg':'<.*/PRP>', 
                    'result':[[('my', 'PRP')]],
                    },
        {'sent':[('designed', 'VBN')], 
                    'reg':'<.*/VBG>', 
                    'result':[],
                    },
        {'sent':[('my', 'PRP'),('test', 'NN'),('string', 'NN'),('will', 'MD'),('be', 'VB'),('useful', 'JJ')], 
                    'reg':'<[^(string|test)]*/.*>', 
                    'result':[[('my', 'PRP')], [('will', 'MD')], [('be', 'VB')], [('useful', 'JJ')]],
                    },
        {'sent':['my','test','string','will','be','useful'], 
                    'reg':'[^<test><will>]+', 
                    'result':[['my'], ['string'], ['be', 'useful']],
                    },
        {'sent':[('my', 'PRP'),('test', 'NN'),('string', 'NN'),('will', 'MD'),('be', 'VB'),('useful', 'JJ')], 
                    'reg':'[<.*e.*/.*>]{2}', 
                    'result':[[('be', 'VB'), ('useful', 'JJ')]],
                    },
        {'sent':[('my', 'PRP'),('test', 'NN'),('string', 'NN'),('will', 'MD'),('be', 'VB'),('useful', 'JJ')], 
                    'reg':'[<test/NN>|<will/MD>]', 
                    'result':[[('test', 'NN')], [('will', 'MD')]],
                    },
        {'sent':['my','test','string','will','be','useful'], 
                    'reg':'<test>', 
                    'result':[['test']],
                    },
        {'sent':['my','test','string','will','be','useful'], 
                    'reg':'<test>.*<be>', 
                    'result':[['test', 'string', 'will', 'be']],
                    },
        {'sent':['my','test','string','will','be','useful'], 
                    'reg':'<.*e.*>{2}', 
                    'result':[['be', 'useful']],
                    },
        {'sent':['my','test','string','will','be','useful'], 
                    'reg': '^<my><test>', 
                    'result':[['my', 'test']],
                    },
        {'sent':['my','test','string','will','be','useful'], 
                    'reg': '<be><useful>$', 
                    'result':[['be', 'useful']],
                    },
        {'sent':'my test string', 
                    'reg': r'\s\w+\s', 
                    'result':[' test '],
                    },
        {'sent':'my test string', 
                    'reg': r'\s\w+\s', 
                    'result':[(2, 6)], 
                    'useindex': True,
                    },
        {'sent':[('my', 'PRP'),('test', 'NN'),('string', 'NN'),('will', 'MD'),('be', 'VB'),('useful', 'JJ')], 
                    'reg': '<[^(string|test)]*/.*>', 
                    'result':[(0, 1), (3, 1), (4, 1), (5, 1)], 
                    'useindex': True,
                    },
        {'sent':'my test string ', 
                    'reg': r'\s\w+\s', 
                    'result':[(2, 6), (7, 8)], 
                    'useindex': True,
                    'overlap': True,
                    },
        {'sent':[('test', 'NN')], 
                    'reg': '<[^(te)]*/.*>', 
                    'result':[[('test', 'NN')]],
                    },
        {'sent':[('not', 'NN')], 
                    'reg': '<not|no/.*>', 
                    'result':[[('not', 'NN')]],
                    },
        {'sent':[('not', 'NN')], 
                    'reg': '<no|not/.*>', 
                    'result':[[('not', 'NN')]],
                    },
        {'sent':'a abc bc', 
                    'reg': '', 
                    'result':[],
                    },
        {'sent':[('a','1'), ('b','2'), ('c','3')], 
                    'reg': '', 
                    'result':[],
                    'useindex': True,
                    },
        {'sent':'ab b', 
                    'reg': '\w{2}', 
                    'result':[(0, 2)],
                    'useindex': True,
                    },
        {'sent':[('not', 'NN')], 
                    'reg': '<not/.*>|<no/.*>', 
                    'result':[[('not', 'NN')]],
                    },
        {'sent':[('appliance', 'NN'), ('retailer', 'NN'), ('and', 'CC'), ('appliance', 'NN'), ('service', 'NN'),], 
                    'reg': '[<.*/NN.*>]+', 
                    'result':[[('appliance', 'NN'), ('retailer', 'NN')], [('appliance', 'NN'), ('service', 'NN')]],
                    },
        {'sent':[('is','VB'), ('a', 'DT')], 
                    'reg': '&[s:s]', 
                    'result':[[('is', 'VB'), ('a', 'DT')]],
                    'var': {'s':['<is/VB.*><a/.*>','<are/VB.*>']},  
                    },
        {'sent':[('are','VB')], 
                    'reg': '&[s:s]', 
                    'result':[[('are', 'VB')]], 
                    'var': {'s':['<is/VB.*><a/.*>','<are/VB.*>']},  
                    },
        {'sent':[('is','VB')], 
                    'reg': '&[s:s]', 
                    'result':[], 
                    'var': {'s':['<is/VB.*><a/.*>','<are/VB.*>']},  
                    },
        {'sent':[('this','DT'), ('is','VB'), ('a', 'VB'), ('are', 'VB'), ('test', 'NN')], 
                    'reg': '&[s:s]+', 
                    'result':[[('is', 'VB'), ('a', 'VB'), ('are', 'VB')]], 
                    'var': {'s':['<is/VB.*><a/.*>','<are/VB.*>']},  
                    },
        {'sent':'this is a test', 
                    'reg': '&[s:s(is|es)].*&[s:s]', 
                    'result':['is is a tes'], 
                    },
        {'sent':'this is a test', 
                    'reg': '&[s:s]', 
                    'result':['is', 'is', 'es'], 
                    'var': {'s':['is', 'es']},  
                    },
        {'sent':'this is a test', 
                    'reg': '&[s:s].*&[s:s]', 
                    'result':['is is a tes'], 
                    'var': {'s':['is', 'es']},  
                    },
        {'sent':'thisises a test', 
                    'reg': '&[s:s]+', 
                    'result':['isises', 'es'], 
                    'var': {'s':['is', 'es']},  
                    },
        {'sent':'thises', 
                    'reg': 'th[^&[s:s]]*es', 
                    'result':[], 
                    'var': {'s':['is', 'es']},  
                    },
        {'sent':'thxxes', 
                    'reg': 'th[^&[s:s]]*es', 
                    'result':['thxxes'], 
                    'var': {'s':['is', 'es']},  
                    },
        {'sent':[('tree', 'NN'), ('in', 'IN')], 
                    'reg': '&[ws:garden]', 
                    'result':[[('tree', 'NN')]], 
                    'var': {'garden':['tree', 'tree cricket']},  
                    },
        {'sent':[('tree', 'NN'), ('in', 'IN'), ('test', 'NN')], 
                    'reg': '&[ws:garden]', 
                    'result':[[('tree', 'NN'), ('in', 'IN')]], 
                    'var': {'garden':['tree', 'tree in', 'tree in cricket']},  
                    },
        {'sent':[('fig', 'NN'), ('tree', 'NN'), ('.', '.')], 
                    'reg': '&[ws:garden]+<.*/NN.*|JJ.*>*', 
                    'result':[[('fig', 'NN'), ('tree', 'NN')]], 
                    'var': {'garden':['fig tree', 'fig', 'tree']},  
                    },
        {'sent':[('is','VB'), ('a', 'DT'),('this', 'DT'),('a', 'DT1'),('this', 'DT'),], 
                    'reg': '&[s:s]+', 
                    'result':[[('is', 'VB'), ('a', 'DT'), ('this', 'DT')], [('this', 'DT')]], 
                    'var': {'s':['<is/VB.*><a/.*>','<is/VB.*>', '<this/DT>']},  
                    },
        {'sent':[('is','VB'), ('a', 'DT'),('b', 'DT'),('this', 'DT'),], 
                    'reg': '[^&[s:s]]+', 
                    'result':[[('a', 'DT'), ('b', 'DT')]], 
                    'var': {'s':['<is/VB.*><a/.*>','<is/VB.*>', '<this/DT>']},  
                    },
        {'sent':[('Ab', 'DT'), ('c', 'NN'), ('dc', 'NN'), ('abc', 'JJ'), ('stricture', 'NN')], 
                    'reg': '&[ws:s]+', 
                    'result':[[('Ab', 'DT'), ('c', 'NN'), ('dc', 'NN')]], 
                    'var': {'s':['ab c', 'dc']},  
                    },
        {'sent':[('Ab', 'DT'), ('c', 'NN'), ('dc', 'NN'), ('abc', 'JJ')], 
                    'reg': '&[ws:s(ab c|dc)][^&[ws:s]]+', 
                    'result':[[('dc', 'NN'), ('abc', 'JJ')]], 
                    },
        {'sent':'testest', 
                    'reg': "t(.*)&[n:a]t(.*)&[n:b]t&[q:a==b]", 
                    'result':['testest'], 
                    },
        {'sent':'testest', 
                    'reg': "t(.*)&[n:a]t(.*)&[n:b]t&[q:a==b]", 
                    'group': True, 
                    'result':[['testest', 'es', 'es']], 
                    },
    ];

def main():
    import argparse
    parser = argparse.ArgumentParser(description='crawl sites')
    parser.add_argument('--i', type=int, default=None, help="test only the ith case")
    args = parser.parse_args()

    if args.i is not None: 
        # test only one case
        global tests
        tests = tests[args.i:args.i+1]

    test_basic_regex(tests)


if __name__ == "__main__":
    main()

    

