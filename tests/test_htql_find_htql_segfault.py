#!/usr/bin/env python
# coding=utf8
"""
Test case to reproduce & troubleshoot segfault in htql when using
  &find_htql with text_words filter on a real page (https://www.bjd.com.cn/).

Run individual steps by passing a step number:
  python test_htql_find_htql_segfault.py 1
  python test_htql_find_htql_segfault.py 2
  ...
Each step is kept in its own process to avoid crashing the test runner.
"""

import sys, os
sys.path.insert(0, r'C:\doc\ava\src')
sys.path.insert(0, r'C:\doc\ava\src\configs\config_beijing')

import htql

TARGET_URL = 'https://www.bjd.com.cn/'

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def fetch_page(url):
    import urllib.request, ssl
    from io import BytesIO
    import gzip
    ctx = ssl.create_default_context()
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_NONE
    req = urllib.request.Request(url)
    req.add_header('User-Agent', 'Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)')
    req.add_header('Accept-encoding', 'gzip')
    resp = urllib.request.urlopen(req, context=ctx, timeout=30)
    raw = resp.read()
    if resp.info().get('Content-Encoding') == 'gzip':
        buf = BytesIO(raw)
        import gzip as gz
        raw = gz.GzipFile(fileobj=buf).read()
    return raw.decode('utf8', errors='ignore')


SIMPLE_EN = """<html><body>
<div><ul>
  <li><a href="/news/1">Breaking news about the economy today worldwide</a></li>
  <li><a href="/news/2">Another long article headline for testing purposes</a></li>
  <li><a href="/news/3">Short</a></li>
  <li><a href="/news/4">Yet another interesting headline in English text words</a></li>
</ul></div>
</body></html>"""

SIMPLE_ZH = """<html><body>
<div><ul>
  <li><a href="/news/1">北京今天发生了一件非常重要的新闻大事</a></li>
  <li><a href="/news/2">关于经济形势的最新分析报告出炉了</a></li>
  <li><a href="/news/3">短</a></li>
  <li><a href="/news/4">全国人民代表大会通过了新的法律法规</a></li>
</ul></div>
</body></html>"""

heuDEPTH_LONG = 3   # value used in LinksRetrieval.findMainLinks

# ---------------------------------------------------------------------------
# Steps
# ---------------------------------------------------------------------------

def step1():
    """Basic <a> extraction on simple English HTML — should work."""
    print("[step1] Basic <a> query on simple English HTML")
    r = htql.query(SIMPLE_EN, '<a> { href=:href; tx=:tx &tx &trim }')
    print("  result count:", len(r))
    for row in r:
        print("  ", row)
    print("  PASS")


def step2():
    """text_words filter on simple English HTML — should work."""
    print("[step2] text_words filter on simple English HTML")
    r = htql.query(SIMPLE_EN, '<a> {a=:href; b=:tx &tx &trim | text_words(b)>5 }')
    print("  result count:", len(r))
    for row in r:
        print("  ", row)
    print("  PASS")


def step3():
    """text_words filter on simple Chinese HTML — may crash."""
    print("[step3] text_words filter on simple Chinese HTML")
    r = htql.query(SIMPLE_ZH, '<a> {a=:href; b=:tx &tx &trim | text_words(b)>5 }')
    print("  result count:", len(r))
    for row in r:
        print("  ", ascii(row))
    print("  PASS")


def step4():
    """&find_htql on simple English HTML (no text_words filter) — should work."""
    print("[step4] find_htql on simple English HTML (no text_words filter)")
    r = htql.query(SIMPLE_EN,
        '<a> {a=:href; b=:tx &tx &trim | strlen(b)>5 } &find_htql(%d)./chr(10)/1' % heuDEPTH_LONG)
    print("  result count:", len(r))
    print("  PASS")


def step5():
    """&find_htql + text_words on simple English HTML — possible crash."""
    print("[step5] find_htql + text_words on simple English HTML")
    r = htql.query(SIMPLE_EN,
        '<a> {a=:href; b=:tx &tx &trim | text_words(b)>5 } &find_htql(%d)./chr(10)/1' % heuDEPTH_LONG)
    print("  result count:", len(r))
    print("  PASS")


def step6():
    """&find_htql + text_words on simple Chinese HTML — likely crash."""
    print("[step6] find_htql + text_words on simple Chinese HTML")
    r = htql.query(SIMPLE_ZH,
        '<a> {a=:href; b=:tx &tx &trim | text_words(b)>5 } &find_htql(%d)./chr(10)/1' % heuDEPTH_LONG)
    print("  result count:", len(r))
    print("  PASS")


def step7():
    """Fetch real bjd.com.cn page and run basic <a> query — should work."""
    print("[step7] Fetch bjd.com.cn + basic <a> query")
    page = fetch_page(TARGET_URL)
    print("  page length:", len(page))
    r = htql.query(page, '<a> { href=:href; tx=:tx &tx &trim }', TARGET_URL)
    print("  result count:", len(r))
    print("  PASS")


def step8():
    """Fetch real bjd.com.cn page + text_words filter (no find_htql) — possible crash."""
    print("[step8] Fetch bjd.com.cn + text_words filter (no find_htql)")
    page = fetch_page(TARGET_URL)
    r = htql.query(page, '<a> {a=:href &url; b=:tx &tx &trim | text_words(b)>5 }', TARGET_URL)
    print("  result count:", len(r))
    print("  PASS")


def step9():
    """Fetch real bjd.com.cn page + full find_htql + text_words — the crashing call."""
    print("[step9] Fetch bjd.com.cn + find_htql + text_words (EXPECTED CRASH)")
    page = fetch_page(TARGET_URL)
    r = htql.query(page,
        '<a> {a=:href &url; b=:tx &tx &trim | text_words(b)>5 } &find_htql(%d)./chr(10)/1' % heuDEPTH_LONG,
        TARGET_URL)
    print("  result count:", len(r))
    print("  PASS (no crash this time)")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

STEPS = [step1, step2, step3, step4, step5, step6, step7, step8, step9]

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
