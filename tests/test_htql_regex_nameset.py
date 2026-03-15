#!/usr/bin/env python
# coding=utf8
"""
Test case to reproduce & troubleshoot segfault in htql.RegEx.setNameSet
which causes LinksRetrieval (and link_page.py CGI) to crash on init.

Run individual steps:
  python test_htql_regex_nameset.py 1
  python test_htql_regex_nameset.py 2
  ...
"""

import sys, os
sys.path.insert(0, r'C:\doc\ava\src')
sys.path.insert(0, r'C:\doc\ava\src\configs\config_beijing')

import htql
from config.config import DB_PATH
from tools.product_functions import read_csv


def load_names():
    first_names = [n[0].lower() for n in read_csv(os.path.join(DB_PATH, 'firstnames.csv'))[1:] if len(n[0]) > 1]
    last_names  = [n[0].lower() for n in read_csv(os.path.join(DB_PATH, 'lastnames.csv'))[1:]]
    return first_names, last_names


# ---------------------------------------------------------------------------
# Steps
# ---------------------------------------------------------------------------

def step1():
    """Load firstnames.csv and lastnames.csv — check counts."""
    first_names, last_names = load_names()
    print(f"  firstnames: {len(first_names)}, lastnames: {len(last_names)}")
    print("  PASS")


def step2():
    """Create htql.RegEx() — basic construction."""
    reg = htql.RegEx()
    print("  htql.RegEx() OK")
    print("  PASS")


def step3():
    """setNameSet('firstname', first_names) — empty list expected (firstnames.csv absent/empty)."""
    first_names, _ = load_names()
    print(f"  firstnames count: {len(first_names)}")
    reg = htql.RegEx()
    reg.setNameSet('firstname', first_names)
    print("  setNameSet firstname OK")
    print("  PASS")


def step4():
    """setNameSet('lastname', last_names[:10]) — small slice."""
    _, last_names = load_names()
    sample = last_names[:10]
    print(f"  sample: {sample}")
    reg = htql.RegEx()
    reg.setNameSet('lastname', sample)
    print("  setNameSet lastname (10) OK")
    print("  PASS")


def step5():
    """setNameSet('lastname', last_names[:100]) — medium slice."""
    _, last_names = load_names()
    print(f"  count: {len(last_names[:100])}")
    reg = htql.RegEx()
    reg.setNameSet('lastname', last_names[:100])
    print("  setNameSet lastname (100) OK")
    print("  PASS")


def step6():
    """setNameSet('lastname', last_names[:500]) — larger slice."""
    _, last_names = load_names()
    print(f"  count: {len(last_names[:500])}")
    reg = htql.RegEx()
    reg.setNameSet('lastname', last_names[:500])
    print("  setNameSet lastname (500) OK")
    print("  PASS")


def step7():
    """setNameSet('lastname', last_names) — full list (known crash)."""
    _, last_names = load_names()
    print(f"  count: {len(last_names)}")
    reg = htql.RegEx()
    reg.setNameSet('lastname', last_names)
    print("  setNameSet lastname (full) OK")
    print("  PASS")


def step8():
    """Both setNameSet calls (firstname + lastname) — as used in contact_parser."""
    first_names, last_names = load_names()
    print(f"  firstnames: {len(first_names)}, lastnames: {len(last_names)}")
    reg = htql.RegEx()
    reg.setNameSet('firstname', first_names)
    print("  firstname OK")
    reg.setNameSet('lastname', last_names)
    print("  lastname OK")
    print("  PASS")


def step9():
    """Full contact_parser init — as called by LinksRetrieval."""
    from tools.contact_parser import contact_parser
    print("  creating contact_parser...")
    cp = contact_parser(DB_PATH)
    print("  contact_parser OK")
    print("  PASS")


def step10():
    """Full LinksRetrieval init — as called by link_page.py CGI."""
    from crawler.LinksRetrieval import LinksRetrieval
    print("  creating LinksRetrieval...")
    r = LinksRetrieval(data_dir=DB_PATH)
    print("  LinksRetrieval OK")
    print("  PASS")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

STEPS = [step1, step2, step3, step4, step5, step6, step7, step8, step9, step10]

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(__doc__)
        print("Available steps:")
        for i, fn in enumerate(STEPS, 1):
            print(f"  {i}: {fn.__doc__.strip()}")
        sys.exit(0)

    step_num = int(sys.argv[1])
    if step_num < 1 or step_num > len(STEPS):
        print(f"Step must be 1-{len(STEPS)}")
        sys.exit(1)

    STEPS[step_num - 1]()
