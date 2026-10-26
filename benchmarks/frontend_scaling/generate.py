#!/usr/bin/env python3
"""Generate C++11 inputs that expose frontend complexity as their size grows."""
import argparse


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('case', choices=['parameter-bases', 'plain-templates',
                                        'independent-defaults', 'chained-defaults',
                                        'nested-chained-defaults'])
    parser.add_argument('size', type=int)
    args = parser.parse_args()
    if args.size < 1:
        parser.error('size must be positive')
    if args.case.endswith('-defaults'):
        parameters = ', '.join(
            f'class T{i}=' + (f'T{i - 1}' if args.case.endswith('chained-defaults') and i else 'int')
            for i in range(args.size))
        print('template<' + parameters + '> struct C {};')
        if args.case == 'nested-chained-defaults':
            print('template<class X> struct Holder { C<> member; };')
            print('static_assert(sizeof(Holder<int>) == 1, "complete");')
        else:
            print('static_assert(sizeof(C<>) == 1, "complete");')
        return
    base = ' : T' if args.case == 'parameter-bases' else ''
    for i in range(args.size):
        print(f'template<class T> struct W{i}{base} {{}};')


if __name__ == '__main__':
    main()
