

;; TODO just copy from move-text.el
;; need to modify to move-xacts

;; find the start of this xact
;; find the end of this xact
;; ==> (ledger-navigate-find-xact-extents pos)  ;; => (start end) ?

;; if move up,
;; find the start of last xact, if any

;; if move down,
;; find the end of next xact, if any
;; (delete dup lines?)

;; if any, copy current xact
;; delete current xact
;; (insert line?)
;; insert to the point found

;; move-text
(defun ledger-move-xact (arg)
  (cond
   ((and mark-active transient-mark-mode)
    (if (> (point) (mark))
        (exchange-point-and-mark))
    (let ((column (current-column))
          (text (delete-and-extract-region (point) (mark))))
      (set-mark (point))
      (forward-line arg)
      (move-to-column column t)
      (insert text)
      (exchange-point-and-mark)
      (setq deactivate-mark nil)))
   (t
    (beginning-of-line)
    (when (or (> arg 0) (not (bobp)))
      (forward-line)
      (when (or (< arg 0) (not (eobp)))
        (transpose-lines arg))
      (forward-line -1)
      (if (> 0 arg)
          (forward-line arg))
      ))))

(defun ledger-move-text-down (arg)
     "Move region (transient-mark-mode active) or current line
  arg lines down."
     (interactive "*p")
     (move-text-internal arg))

(defun ledger-move-text-up (arg)
     "Move region (transient-mark-mode active) or current line
  arg lines up."
     (interactive "*p")
     (move-text-internal (- arg)))

;;; (global-set-key (kbd "s-<up>") 'ledger-move-xact-up)
;;; (global-set-key (kbd "s-<down>") 'ledger-move-xact-down)

(provide 'ledger-move)


