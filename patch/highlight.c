static void
#if EMOJI_HIGHLIGHT_PATCH
drawhighlights(struct item *item, char *output, int x, int y, int maxw)
#else
drawhighlights(struct item *item, int x, int y, int maxw)
#endif // EMOJI_HIGHLIGHT_PATCH
{
	char restorechar, tokens[sizeof text], *highlight,  *token;
	int indentx, highlightlen;

	#if FUZZYMATCH_PATCH
	char c;
	int i, indent;
	int utf8charlen;
	#endif // FUZZYMATCH_PATCH

	#if EMOJI_HIGHLIGHT_PATCH
	char *itemtext = output;
	#elif TSV_PATCH && !SEPARATOR_PATCH
	char *itemtext = item->stext;
	#else
	char *itemtext = item->text;
	#endif // EMOJI_HIGHLIGHT_PATCH | TSV_PATCH

	if (!(strlen(itemtext) && strlen(text)))
		return;

	/* Do not highlight items scheduled for output */
	#if MULTI_SELECTION_PATCH
	if (issel(item->id))
		return;
	#else
	if (item->out)
		return;
	#endif // MULTI_SELECTION_PATCH

	drw_setscheme(drw, scheme[item == sel ? SchemeSelHighlight : SchemeNormHighlight]);

	#if FUZZYMATCH_PATCH
	if (fuzzy) {
		for (i = 0, highlight = itemtext; *highlight && text[i];) {
			utf8charlen = utf8len(highlight);
			#if FUZZYMATCH_PATCH
			if (!fstrncmp(&(*highlight), &text[i], utf8charlen))
			#else
			if (*highlight == text[i])
			#endif // FUZZYMATCH_PATCH
			{
				/* get indentation */
				c = *highlight;
				*highlight = '\0';
				indent = TEXTW(itemtext) - lrpad;
				*highlight = c;

				/* highlight character */
				c = highlight[utf8charlen];
				highlight[utf8charlen] = '\0';
				drw_text(
					drw,
					x + indent + (lrpad / 2),
					y,
					MIN(maxw - indent - lrpad, TEXTW(highlight) - lrpad),
					bh, 0, highlight, 0
					#if PANGO_PATCH
					, True
					#endif // PANGO_PATCH
				);
				highlight[utf8charlen] = c;
				i += utf8charlen;
			}
			highlight++;
		}
		return;
	}
	#endif // FUZZYMATCH_PATCH

	strcpy(tokens, text);
	for (token = strtok(tokens, " "); token; token = strtok(NULL, " ")) {
		highlight = fstrstr(itemtext, token);
		while (highlight) {
			// Move item str end, calc width for highlight indent, & restore
			highlightlen = highlight - itemtext;
			restorechar = *highlight;
			itemtext[highlightlen] = '\0';
			indentx = TEXTW(itemtext);
			itemtext[highlightlen] = restorechar;

			// Move highlight str end, draw highlight, & restore
			restorechar = highlight[strlen(token)];
			highlight[strlen(token)] = '\0';
			if (indentx - (lrpad / 2) - 1 < maxw)
				drw_text(
					drw,
					x + indentx - (lrpad / 2) - 1,
					y,
					MIN(maxw - indentx, TEXTW(highlight) - lrpad),
					bh, 0, highlight, 0
					#if PANGO_PATCH
					, True
					#endif // PANGO_PATCH
				);
			highlight[strlen(token)] = restorechar;

			if (strlen(highlight) - strlen(token) < strlen(token))
				break;
			highlight = fstrstr(highlight + strlen(token), token);
		}
	}
}
