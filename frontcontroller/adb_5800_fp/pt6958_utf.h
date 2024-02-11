/**************************************************
 *
 * Conversion table UTF-8 -> ASCII for PT6958
 *
 * Replaces accented letter with the non-accented
 * ASCII variants as segment pattern
 */
unsigned char PT6958_UTF_C2[64] =
{
	// table: 0xc2 0xXX (symbols) -> (LED segment pattern)
	0x00,	// 0x80, nonprintable control
	0x00,	// 0x81, nonprintable control
	0x00,	// 0x82, nonprintable control
	0x00,	// 0x83, nonprintable control
	0x00,	// 0x84, nonprintable control
	0x00,	// 0x85, nonprintable control
	0x00,	// 0x86, nonprintable control
	0x00,	// 0x87, nonprintable control
	0x00,	// 0x88, nonprintable control
	0x00,	// 0x89, nonprintable control
	0x00,	// 0x8a, nonprintable control
	0x00,	// 0x8b, nonprintable control
	0x00,	// 0x8c, nonprintable control
	0x00,	// 0x8d, nonprintable control
	0x00,	// 0x8e, nonprintable control
	0x00,	// 0x8f, nonprintable control

	0x00,	// 0x90, nonprintable control
	0x00,	// 0x91, nonprintable control
	0x00,	// 0x92, nonprintable control
	0x00,	// 0x93, nonprintable control
	0x00,	// 0x94, nonprintable control
	0x00,	// 0x95, nonprintable control
	0x00,	// 0x96, nonprintable control
	0x00,	// 0x97, nonprintable control
	0x00,	// 0x98, nonprintable control
	0x00,	// 0x99, nonprintable control
	0x00,	// 0x9a, nonprintable control
	0x00,	// 0x9b, nonprintable control
	0x00,	// 0x9c, nonprintable control
	0x00,	// 0x9d, nonprintable control
	0x00,	// 0x9e, nonprintable control
	0x00,	// 0x9f, nonprintable control

	0x00,	// 0xa0, no break space
	0x06,	// 0xa1, inverted exclamation mark
	0x58,	// 0xa2, cent sign
	0x3d,	// 0xa3, pound sign
	0x64,	// 0xa4, currency sign
	0x66,	// 0xa5, yen sign
	0x06,	// 0xa6, broken bar
	0x67,	// 0xa7, section sign
	0x68,	// 0xa8, diaeresis
	0x69,	// 0xa9, copyright sign
	0xa6,	// 0xaa, feminine ordinal indicator
	0xae,	// 0xab, left pointing double angle quotation mark
	0x01,	// 0xac, not sign
	0x40,	// 0xad, soft hyphen
	0x50,	// 0xae, registered sign
	0x01,	// 0xaf, macron

	0x63,	// 0xb0, degree sign
	0x68,	// 0xb1, plus-minus sign
	0x5b,	// 0xb2, superscript two
	0x33,	// 0xb3, superscript three
	0x20,	// 0xb4, acute accent
	0x1c,	// 0xb5, micro sign
	0x67,	// 0xb6, pilcrow sign
	0x40,	// 0xb7, middle dot
	0x84,	// 0xb8, cedilla
	0x06,	// 0xb9, superscript one
	0x00,	// 0xba, masculine ordinal indicator
	0x00,	// 0xbb, right pointing double angle quotation mark
	0x34,	// 0xbc, vulgar fraction one quarter
	0x32,	// 0xbd, vulgar fraction one half
	0x33,	// 0xbe, vulgar fraction three quarters
	0x5a	// 0xbf, inverted question mark
};

unsigned char PT6958_UTF_C3[64] =
{
	// table: 0xc3 0xXX -> non accented ASCII (LED segment pattern)
	0x77,	// 0x80, capital A with accent grave
	0x77,	// 0x81, capital A with accent acute
	0x77,	// 0x82, capital A with accent circonflexe
	0x77,	// 0x83, capital A with accent tilde
	0x77,	// 0x84, capital A with accent diaeresis //Ä
	0x77,	// 0x85, capital A with accent ring above
	0x77,	// 0x86, capital letter AE
	0x39,	// 0x87, capital C with cedille
	0x79,	// 0x88, capital E with accent grave
	0x79,	// 0x89, capital E with accent acute
	0x79,	// 0x8a, capital E with accent circonflexe
	0x79,	// 0x8b, capital E with accent diaeresis
	0x06,	// 0x8c, capital I with accent grave
	0x06,	// 0x8d, capital I with accent acute
	0x06,	// 0x8e, capital I with accent circonflexe
	0x06,	// 0x8f, capital I with accent diaeresis

	0x5e,	// 0x90, capital D with dash
	0x37,	// 0x91, capital N with accent tilde
	0x3f,	// 0x92, capital O with accent grave
	0x3f,	// 0x93, capital O with accent acute
	0x3f,	// 0x94, capital O with accent circonflexe
	0x3f,	// 0x95, capital O with accent tilde
	0x3f,	// 0x96, capital O with accent diaeresis //Ö
	0x76,	// 0x97, multiplication sign
	0x3f,	// 0x98, capital O with stroke
	0x3e,	// 0x99, capital U with accent grave
	0x3e,	// 0x9a, capital U with accent acute
	0x3e,	// 0x9b, capital U with accent circonflexe
	0x3e,	// 0x9c, capital U with accent diaeresis //Ü
	0x6b,	// 0x9d, capital Y with accent acute
	0x00,	// 0x9e, capital thorn
	0x6d,	// 0x9f, ringel-S  //ß

	0x5f,	// 0xa0, small a with accent grave
	0x5f,	// 0xa1, small a with accent acute
	0x5f,	// 0xa2, small a with accent circonflexe
	0x5f,	// 0xa3, small a with accent tilde
	0x5f,	// 0xa4, small a with accent diaeresis //ä
	0x5f,	// 0xa5, small a with accent ring above
	0x5f,	// 0xa6, small letter AE
	0x58,	// 0xa7, small c with cedille
	0x7b,	// 0xa8, small e with accent grave
	0x7b,	// 0xa9, small e with accent acute
	0x7b,	// 0xaa, small e with accent circonflexe
	0x7b,	// 0xab, small e with accent diaeresis
	0x04,	// 0xac, small i with accent grave
	0x04,	// 0xad, small i with accent acute
	0x04,	// 0xae, small i with accent circonflexe
	0x04,	// 0xaf, small i with accent diaeresis

	0x00,	// 0xb0, small letter ETH
	0x54,	// 0xb1, small n with accent tilde
	0x5c,	// 0xb2, small o with accent grave
	0x5c,	// 0xb3, small o with accent acute
	0x5c,	// 0xb4, small o with accent circonflexe
	0x5c,	// 0xb5, small o with accent tilde
	0x5c,	// 0xb6, small o with accent diaeresis //ö
	0x40,	// 0xb7, division sign
	0x5c,	// 0xb8, small o with stroke
	0x1c,	// 0xb9, small u with accent grave
	0x1c,	// 0xba, small u with accent acute
	0x1c,	// 0xbb, small u with accent circonflexe
	0x1c,	// 0xbc, small u with accent diaeresis //ü
	0x6b,	// 0xbd, small y with accent acute
	0x00,	// 0xbe, small letter thorn
	0x6b	// 0xbf, small y with accent diaresis
};

unsigned char PT6958_UTF_C4[64] =
{
	// table: 0xc4 0xXX -> non accented ASCII (LED segment pattern)
	0x77,	// 0x80, capital A with accent macron
	0x5f,	// 0x81, small a with accent macron
	0x77,	// 0x82, capital A with accent breve
	0x5f,	// 0x83, small a with accent breve
	0x77,	// 0x84, capital A with accent ogonek (polish 
	0x51,	// 0x85, small a with accent ogonek (polish ac
	0x39,	// 0x86, capital C with accent acute (polish a
	0x58,	// 0x87, small c with accent acute (polish acc
	0x39,	// 0x88, capital C with accent circonflexe
	0x58,	// 0x89, small c with accent circonflexe
	0x39,	// 0x8a, capital C with dot above
	0x58,	// 0x8b, small c with dot above
	0x39,	// 0x8c, capital C with accent caron
	0x58,	// 0x8d, small c with accent caron
	0x5e,	// 0x8e, capital D with accent caron
	0x5e,	// 0x8f, small d with accent caron

	0x5e,	// 0x90, capital D with stroke
	0x5e,	// 0x91, small d with stroke
	0x79,	// 0x92, capital E with accent macron
	0x7b,	// 0x93, small e with accent macron
	0x79,	// 0x94, capital E with accent breve
	0x7b,	// 0x95, small e with accent breve
	0x79,	// 0x96, capital E with dot above
	0x7b,	// 0x97, small e with dot above
	0x79,	// 0x98, capital E with accent ogonek (polish 
	0x7b,	// 0x99, small e with accent ogonek (polish ac
	0x79,	// 0x9a, capital E with accent caron
	0x7b,	// 0x9b, small e with accent caron
	0x3d,	// 0x9c, capital G with accent circonflexe
	0x6f,	// 0x9d, small g with accent circonflexe
	0x3d,	// 0x9e, capital G with accent breve
	0x6f,	// 0x9f, small g with accent breve

	0x3d,	// 0xa0, capital G with dot above
	0x6f,	// 0xa1, small g with dot above
	0x3d,	// 0xa2, capital G with cedille
	0x6f,	// 0xa3, small g with cedille
	0x76,	// 0xa4, capital H with accent circonflexe
	0x7c,	// 0xa5, small h with accent circonflexe
	0x76,	// 0xa6, capital H with stroke
	0x7c,	// 0xa7, small h with stroke
	0x06,	// 0xa8, capital I with tilde
	0x04,	// 0xa9, small i with tilde
	0x06,	// 0xaa, capital I with accent macron
	0x04,	// 0xab, small i with accent macron
	0x06,	// 0xac, capital I with accent breve
	0x04,	// 0xad, small i with accent breve
	0x06,	// 0xae, capital I with accent ogonek
	0x04,	// 0xaf, small i with accent ogonek

	0x06,	// 0xb0, capital I with dot above
	0x04,	// 0xb1, small i without dot
	0x6b,	// 0xb2, capital IJ
	0x6b,	// 0xb3, small ij
	0x1b,	// 0xb4, capital J with accent circonflexe
	0x1b,	// 0xb5, small j with accent circonflexe
	0x7c,	// 0xb6, capital K with cedille
	0x7c,	// 0xb7, small k with cedille
	0x00,	// 0xb8, small letter kra
	0x38,	// 0xb9, capital L with accent acute
	0x06,	// 0xba, small l with accent acute
	0x38,	// 0xbb, capital L with cedille
	0x06,	// 0xbc, small l with  cedille
	0x38,	// 0xbd, capital L with accent caron
	0x06,	// 0xbe, small l with accent caron
	0x38	// 0xbf, capital L with middle dot
};

unsigned char PT6958_UTF_C5[64] =
{
	// table: 0xc5 0xXX -> non accented ASCII (LED segment pattern)
	0x06,	// 0x80, small l with middle dot
	0x38,	// 0x81, capital L with stroke (polish accented L)
	0x06,	// 0x82, small l with stroke (polish accented l)
	0x37,	// 0x83, capital N with accent acute (polish accented N)
	0x54,	// 0x84, small n with accent acute (polish accented n)
	0x37,	// 0x85, capital N with cedille
	0x54,	// 0x86, small n with cedille
	0x37,	// 0x87, capital N with accent caron
	0x54,	// 0x88, small n with accent caron
	0x54,	// 0x89, small n with apostrofe in front
	0x00,	// 0x8a, capital letter eng
	0x00,	// 0x8b, small letter eng
	0x3f,	// 0x8c, capital O with macron
	0x5c,	// 0x8d, small o with macron
	0x3f,	// 0x8e, capital O with breve
	0x5c,	// 0x8f, small o with breve

	0x3f,	// 0x90, capital O with double accent acute
	0x5c,	// 0x91, small o with double accent acute
	0x3f,	// 0x92, capital OE
	0x5c,	// 0x93, small oe
	0x50,	// 0x94, capital R with accent acute
	0x50,	// 0x95, small r with accent acute
	0x50,	// 0x96, capital R with accent acute
	0x50,	// 0x97, small r with accent acute
	0x50,	// 0x98, capital R with accent caron
	0x50,	// 0x99, small r with accent caron
	0x6d,	// 0x9a, capital S with accent acute (polish accented S)
	0x6d,	// 0x9b, small s with accent acute ( polish accented s)
	0x6d,	// 0x9c, capital S with accent circonflexe
	0x6d,	// 0x9d, small s with accent circonflexe
	0x6d,	// 0x9e, capital S with cedille
	0x6d,	// 0x9f, small s with cedille

	0x6d,	// 0xa0, capital S with caron
	0x6d,	// 0xa1, small s with caron
	0x58,	// 0xa2, capital T with cedille
	0x58,	// 0xa3, small t with cedille
	0x58,	// 0xa4, capital T with caron
	0x58,	// 0xa5, small t with caron
	0x58,	// 0xa6, capital T with stroke
	0x58,	// 0xa7, small t with stroke
	0x3e,	// 0xa8, capital U with tilde
	0x1c,	// 0xa9, small u with tilde
	0x3e,	// 0xaa, capital U with macron
	0x1c,	// 0xab, small u with macron
	0x3e,	// 0xac, capital U with breve
	0x1c,	// 0xad, small u with breve
	0x3e,	// 0xae, capital U with ring above
	0x1c,	// 0xaf, small u with ring above

	0x3e,	// 0xb0, capital U with double accent acute
	0x1c,	// 0xb1, small u with double accent acute
	0x3e,	// 0xb2, capital U with ogonek
	0x1c,	// 0xb3, small u with ogonek
	0x3e,	// 0xb4, capital W with accent circonflexe
	0x1c,	// 0xb5, small w with accent circonflexe
	0x6b,	// 0xb6, capital Y with accent circonflexe
	0x6b,	// 0xb7, small y with accent circonflexe
	0x3e,	// 0xb8, capital W with accent diaeresis
	0x5b,	// 0xb9, capital Z with accent acute (polish accented Zi)
	0x5b,	// 0xba, small z with accent acute (polish accented zi)
	0x5b,	// 0xbb, capital Z with dot above (polish accented Z)
	0x5b,	// 0xbc, small z with dot above (polish accented z)
	0x5b,	// 0xbd, capital Z with accent caron
	0x5b,	// 0xbe, small z with accent caron
	0x6d	// 0xbf, small long s
};

unsigned char PT6958_UTF_D0[64] =
// Cyrillic, currently not supported
{
	// table: 0xd0 0xXX -> non accented ASCII (LED segment pattern)
	0x00,	// 0x80, 
	0x00,	// 0x81, 
	0x00,	// 0x82, 
	0x00,	// 0x83, 
	0x00,	// 0x84, 
	0x00,	// 0x85, 
	0x00,	// 0x86, 
	0x00,	// 0x87, 
	0x00,	// 0x88, 
	0x00,	// 0x89, 
	0x00,	// 0x8a, 
	0x00,	// 0x8b, 
	0x00,	// 0x8c, 
	0x00,	// 0x8d, 
	0x00,	// 0x8e, 
	0x00,	// 0x8f, 

	0x00,	// 0x90, 
	0x00,	// 0x91, 
	0x00,	// 0x92, 
	0x00,	// 0x93, 
	0x00,	// 0x94, 
	0x00,	// 0x95, 
	0x00,	// 0x96, 
	0x00,	// 0x97, 
	0x00,	// 0x98, 
	0x00,	// 0x99, 
	0x00,	// 0x9a, 
	0x00,	// 0x9b, 
	0x00,	// 0x9c, 
	0x00,	// 0x9d, 
	0x00,	// 0x9e, 
	0x00,	// 0x9f, 

	0x00,	// 0xa0, 
	0x00,	// 0xa1, 
	0x00,	// 0xa2, 
	0x00,	// 0xa3, 
	0x00,	// 0xa4, 
	0x00,	// 0xa5, 
	0x00,	// 0xa6, 
	0x00,	// 0xa7, 
	0x00,	// 0xa8, 
	0x00,	// 0xa9, 
	0x00,	// 0xaa, 
	0x00,	// 0xab, 
	0x00,	// 0xac, 
	0x00,	// 0xad, 
	0x00,	// 0xae, 
	0x00,	// 0xaf, 

	0x00,	// 0xb0, 
	0x00,	// 0xb1, 
	0x00,	// 0xb2, 
	0x00,	// 0xb3, 
	0x00,	// 0xb4, 
	0x00,	// 0xb5, 
	0x00,	// 0xb6, 
	0x00,	// 0xb7, 
	0x00,	// 0xb8, 
	0x00,	// 0xb9, 
	0x00,	// 0xba, 
	0x00,	// 0xbb, 
	0x00,	// 0xbc, 
	0x00,	// 0xbd, 
	0x00,	// 0xbe, 
	0x00	// 0xbf,
};

unsigned char PT6958_UTF_D1[64] =
// Cyrillic, currently not supported
{
	//  table: 0xd1 0xXX -> non accented ASCII (LED segment pattern)
	0x00,	// 0x80, 
	0x00,	// 0x81, 
	0x00,	// 0x82, 
	0x00,	// 0x83, 
	0x00,	// 0x84, 
	0x00,	// 0x85, 
	0x00,	// 0x86, 
	0x00,	// 0x87, 
	0x00,	// 0x88, 
	0x00,	// 0x89, 
	0x00,	// 0x8a, 
	0x00,	// 0x8b, 
	0x00,	// 0x8c, 
	0x00,	// 0x8d, 
	0x00,	// 0x8e, 
	0x00,	// 0x8f, 

	0x00,	// 0x90, 
	0x00,	// 0x91, 
	0x00,	// 0x92, 
	0x00,	// 0x93, 
	0x00,	// 0x94, 
	0x00,	// 0x95, 
	0x00,	// 0x96, 
	0x00,	// 0x97, 
	0x00,	// 0x98, 
	0x00,	// 0x99, 
	0x00,	// 0x9a, 
	0x00,	// 0x9b, 
	0x00,	// 0x9c, 
	0x00,	// 0x9d, 
	0x00,	// 0x9e, 
	0x00,	// 0x9f, 

	0x00,	// 0xa0, 
	0x00,	// 0xa1, 
	0x00,	// 0xa2, 
	0x00,	// 0xa3, 
	0x00,	// 0xa4, 
	0x00,	// 0xa5, 
	0x00,	// 0xa6, 
	0x00,	// 0xa7, 
	0x00,	// 0xa8, 
	0x00,	// 0xa9, 
	0x00,	// 0xaa, 
	0x00,	// 0xab, 
	0x00,	// 0xac, 
	0x00,	// 0xad, 
	0x00,	// 0xae, 
	0x00,	// 0xaf, 

	0x00,	// 0xb0, 
	0x00,	// 0xb1, 
	0x00,	// 0xb2, 
	0x00,	// 0xb3, 
	0x00,	// 0xb4, 
	0x00,	// 0xb5, 
	0x00,	// 0xb6, 
	0x00,	// 0xb7, 
	0x00,	// 0xb8, 
	0x00,	// 0xb9, 
	0x00,	// 0xba, 
	0x00,	// 0xbb, 
	0x00,	// 0xbc, 
	0x00,	// 0xbd, 
	0x00,	// 0xbe, 
	0x00	// 0xbf,
};
// vim:ts=4
