# TestD2DTextRender

<img src="media/screenshot.png" width="80%">

Windows 11において、Notepad.exeのテキストレンダリング品質に疑問を覚えた。特に日本語の漢字かな文字で顕著なジャギーを認めた。そこで、Direct2D/DirectWrite環境におけるテキストレンダリングの方法について調査した。これは実験であり、実用的な実装ではない。  

On Windows 11, the text rendering quality of Notepad.exe was questionable. In particular, I observed significant jaggies in Japanese Kanji/kana characters. Therefore, I investigated the text rendering method in the Direct2D/DirectWrite environment. This is an experiment, not a practical implementation.

## Requirement

* Visual Studio 2019
* Windows Template Library (WTL) 10

## References

* [RichEditD2D Window Controls](https://devblogs.microsoft.com/math-in-office/richeditd2d-window-controls/)

## Written by

[yu2924](https://twitter.com/yu2924)

## License

CC0 1.0 Universal
