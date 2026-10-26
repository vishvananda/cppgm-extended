#!/usr/bin/env python3
"""Generate C++11 inputs that expose frontend complexity as their size grows."""
import argparse


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('case', choices=['parameter-bases', 'plain-templates'])
    parser.add_argument('size', type=int)
    args = parser.parse_args()
    if args.size < 1:
        parser.error('size must be positive')
    base = ' : T' if args.case == 'parameter-bases' else ''
    for i in range(args.size):
        print(f'template<class T> struct W{i}{base} {{}};')


if __name__ == '__main__':
    main()
