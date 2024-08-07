static void
buttonpress(XEvent *e)
{
	struct item *item;
	XButtonPressedEvent *ev = &e->xbutton;
	int x = 0, y = 0, h = bh, w;
	#if GRID_PATCH
	int i, cols;
	#endif // GRID_PATCH

	if (ev->window != win)
		return;

	/* right-click: exit */
	if (ev->button == Button3)
		exit(1);

	if (prompt && *prompt)
		x += promptw;

	/* input field */
	w = (lines > 0 || !matches) ? mw - x : inputw;

	/* left-click on input: clear input,
	 * NOTE: if there is no left-arrow the space for < is reserved so
	 *       add that to the input width */
	#if SYMBOLS_PATCH
	if (ev->button == Button1 &&
	   ((lines <= 0 && ev->x >= 0 && ev->x <= x + w +
	   ((!prev || !curr->left) ? TEXTW(symbol_1) : 0)) ||
	   (lines > 0 && ev->y >= y && ev->y <= y + h))) {
		insert(NULL, -cursor);
		drawmenu();
		return;
	}
	#else
	if (ev->button == Button1 &&
	   ((lines <= 0 && ev->x >= 0 && ev->x <= x + w +
	   ((!prev || !curr->left) ? TEXTW("<") : 0)) ||
	   (lines > 0 && ev->y >= y && ev->y <= y + h))) {
		insert(NULL, -cursor);
		drawmenu();
		return;
	}
	#endif // SYMBOLS_PATCH
	/* middle-mouse click: paste selection */
	if (ev->button == Button2) {
		XConvertSelection(dpy, (ev->state & ShiftMask) ? clip : XA_PRIMARY,
		                  utf8, utf8, win, CurrentTime);
		drawmenu();
		return;
	}
	/* scroll up */
	if (ev->button == Button4 && prev) {
		sel = curr = prev;
		calcoffsets();
		drawmenu();
		return;
	}
	/* scroll down */
	if (ev->button == Button5 && next) {
		sel = curr = next;
		calcoffsets();
		drawmenu();
		return;
	}
	if (ev->button != Button1)
		return;
	if (ev->state & ~ControlMask)
		return;
	if (lines > 0) {
		#if GRID_PATCH
		cols = columns ? columns : 1;
		for (i = 0, item = curr; item != next; item = item->right, i++) {
			if (
				(ev->y >= y + ((i % lines) + 1) * bh) && // line y start
				(ev->y <= y + ((i % lines) + 2) * bh) && // line y end
				(ev->x >= x + ((i / lines) * (w / cols))) && // column x start
				(ev->x <= x + ((i / lines + 1) * (w / cols))) // column x end
			) {
				#if !MULTI_SELECTION_PATCH
				puts(item->text);
				#endif // MULTI_SELECTION_PATCH
				if (!(ev->state & ControlMask)) {
					#if MULTI_SELECTION_PATCH
					sel = item;
					selsel();
					printsel(ev->state);
					#endif // MULTI_SELECTION_PATCH
					exit(0);
				}
				sel = item;
				if (sel) {
					#if MULTI_SELECTION_PATCH
					selsel();
					#else
					sel->out = 1;
					#endif // MULTI_SELECTION_PATCH
					drawmenu();
				}
				return;
			}
		}
		#else
		/* vertical list: (ctrl)left-click on item */
		for (item = curr; item != next; item = item->right) {
			y += h;
			if (ev->y >= y && ev->y <= (y + h)) {
				#if !MULTI_SELECTION_PATCH
				puts(item->text);
				#endif // MULTI_SELECTION_PATCH
				if (!(ev->state & ControlMask)) {
					#if MULTI_SELECTION_PATCH
					sel = item;
					selsel();
					printsel(ev->state);
					#endif // MULTI_SELECTION_PATCH
					exit(0);
				}
				sel = item;
				if (sel) {
					#if MULTI_SELECTION_PATCH
					selsel();
					#else
					sel->out = 1;
					#endif // MULTI_SELECTION_PATCH
					drawmenu();
				}
				return;
			}
		}
		#endif // GRID_PATCH
	} else if (matches) {
		/* left-click on left arrow */
		x += inputw;
		#if SYMBOLS_PATCH
		w = TEXTW(symbol_1);
		#else
		w = TEXTW("<");
		#endif // SYMBOLS_PATCH
		if (prev && curr->left) {
			if (ev->x >= x && ev->x <= x + w) {
				sel = curr = prev;
				calcoffsets();
				drawmenu();
				return;
			}
		}
		/* horizontal list: (ctrl)left-click on item */
		for (item = curr; item != next; item = item->right) {
			x += w;
			#if SYMBOLS_PATCH
			w = MIN(TEXTW(item->text), mw - x - TEXTW(symbol_2));
			#else
			w = MIN(TEXTW(item->text), mw - x - TEXTW(">"));
			#endif // SYMBOLS_PATCH
			if (ev->x >= x && ev->x <= x + w) {
				#if !MULTI_SELECTION_PATCH
				puts(item->text);
				#endif // MULTI_SELECTION_PATCH
				if (!(ev->state & ControlMask)) {
					#if MULTI_SELECTION_PATCH
					sel = item;
					selsel();
					printsel(ev->state);
					#endif // MULTI_SELECTION_PATCH
					exit(0);
				}
				sel = item;
				if (sel) {
					#if MULTI_SELECTION_PATCH
					selsel();
					#else
					sel->out = 1;
					#endif // MULTI_SELECTION_PATCH
					drawmenu();
				}
				return;
			}
		}
		/* left-click on right arrow */
		#if SYMBOLS_PATCH
		w = TEXTW(symbol_2);
		#else
		w = TEXTW(">");
		#endif // SYMBOLS_PATCH
		x = mw - w;
		if (next && ev->x >= x && ev->x <= x + w) {
			sel = curr = next;
			calcoffsets();
			drawmenu();
			return;
		}
	}
}

#if MOTION_SUPPORT_PATCH
static void
motionevent(XButtonEvent *ev)
{
	struct item *item;
	int x = 0, y = 0, w;
	#if GRID_PATCH
	int i, cols;
	#endif // GRID_PATCH

	if (ev->window != win || matches == 0)
		return;

	if (prompt && *prompt)
		x += promptw;

	if (lines > 0) {
		/* input field */
		w = mw - x;
		#if GRID_PATCH
		cols = columns ? columns : 1;
		/* grid view or vertical list */
		for (i = 0, item = curr; item != next; item = item->right, i++) {
			if (
				(ev->y >= y + ((i % lines) + 1) * bh) && // line y start
				(ev->y <= y + ((i % lines) + 2) * bh) && // line y end
				(ev->x >= x + ((i / lines) * (w / cols))) && // column x start
				(ev->x <= x + ((i / lines + 1) * (w / cols))) // column x end
			) {
				sel = item;
				calcoffsets();
				drawmenu();
				break;
			}
		}
		#else
		/* vertical list */
		w = mw - x;
		for (item = curr; item != next; item = item->right) {
			y += bh;
			if (ev->y >= y && ev->y <= (y + bh)) {
				sel = item;
				calcoffsets();
				drawmenu();
				break;
			}
		}
		#endif // GRID_PATCH
		return;
	}

	/* left-click on left arrow */
	x += inputw;
	#if SYMBOLS_PATCH
	w = TEXTW(symbol_1);
	#else
	w = TEXTW("<");
	#endif // SYMBOLS_PATCH
	/* horizontal list */
	for (item = curr; item != next; item = item->right) {
		x += w;
		#if SYMBOLS_PATCH
		w = MIN(TEXTW(item->text), mw - x - TEXTW(symbol_2));
		#else
		w = MIN(TEXTW(item->text), mw - x - TEXTW(">"));
		#endif // SYMBOLS_PATCH
		if (ev->x >= x && ev->x <= x + w) {
			sel = item;
			calcoffsets();
			drawmenu();
			break;
		}
	}
}
#endif
